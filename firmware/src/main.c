/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdint.h>
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <stdio.h>
#include "buffer.h"
#include "ringbuffer.h"
#include "sensor.h"
#include "app_config.h"
#if STREAM_FORMAT_IS(SMLSS)
#include "ssi_comms.h"
#endif //STREAM_FORMAT_IS(SMLSS)
// *****************************************************************************
// *****************************************************************************
// Section: Platform specific includes
// *****************************************************************************
// *****************************************************************************
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global variables
// *****************************************************************************
// *****************************************************************************

/* Must be large enough to hold the connect/disconnect strings from SensiML DCL */
static uint8_t _uartRxBuffer_data[128];
static ringbuffer_t uartRxBuffer;

static volatile uint32_t tickcounter = 0;
static volatile unsigned int tickrate = 0;

static struct sensor_device_t sensor;
static struct sensor_buffer_t snsr_buffer;

// *****************************************************************************
// *****************************************************************************
// Section: Platform specific stub definitions
// *****************************************************************************
// *****************************************************************************
void SERCOM5_Handler() {
    uint8_t *ptr;
    if (UART_IsRxReady() && ringbuffer_get_write_buffer(&uartRxBuffer, &ptr)) {
        *ptr = UART_RX_DATA;
        ringbuffer_advance_write_index(&uartRxBuffer, 1);
    }
}

static size_t __attribute__(( unused )) UART_Write(uint8_t *ptr, const size_t nbytes) {
    return SERCOM5_USART_Write(ptr, nbytes) ? nbytes : 0;
}

// *****************************************************************************
// *****************************************************************************
// Section: Generic stub definitions
// *****************************************************************************
// *****************************************************************************

static size_t __attribute__(( unused )) UART_Read(uint8_t *ptr, const size_t nbytes) {
    return ringbuffer_read(&uartRxBuffer, ptr, nbytes);
}

void Ticker_Callback() {
    static unsigned int mstick = 0;

    ++tickcounter;
    if (tickrate == 0) {
        mstick = 0;
    }
    else if (++mstick == tickrate) {
        LED_STATUS_Toggle();
        mstick = 0;
    }
}

uint64_t read_timer_ms(void) {
    return tickcounter;
}

uint64_t read_timer_us(void) {
    return tickcounter * 1000U + (uint32_t) TC_TimerGet_us();
}

void sleep_ms(uint32_t ms) {
    uint32_t t0 = read_timer_ms();
    while ((read_timer_ms() - t0) < ms) { };
}

void sleep_us(uint32_t us) {
    uint32_t t0 = read_timer_us();
    while ((read_timer_us() - t0) < us) { };
}

// For handling read of the sensor data
void SNSR_ISR_HANDLER() {    
    /* Check if any errors we've flagged have been acknowledged */
    if (sensor.status != SNSR_STATUS_OK) {
        return;
    }
    
    buffer_data_t *ptr;
    if (buffer_get_write_buffer(&snsr_buffer, &ptr) \
            && (sensor.status = sensor_read(&sensor, ptr)) == SNSR_STATUS_OK) {
        buffer_advance_write_index(&snsr_buffer, 1);
    }
    else {
        snsr_buffer.overrun = true;
    }
}

#if STREAM_FORMAT_IS(SMLSS)
static char json_config_str[SML_MAX_CONFIG_STRLEN];

size_t ssi_build_json_config(char json_config_str[], size_t maxlen)
{
    size_t written=0;
    size_t snsr_index = 0;

    written += snprintf(json_config_str, maxlen, 
            "{\"version\":%d"
            ",\"sample_rate\":%d"
            ",\"samples_per_packet\":%d"
            ",\"column_location\":{"
            , SSI_JSON_CONFIG_VERSION, SNSR_SAMPLE_RATE_IN_HZ, SNSR_SAMPLES_PER_PACKET);
#if SNSR_USE_ACCEL_X
    written += snprintf(json_config_str+written, maxlen-written, "\"AccelerometerX\":%d,", snsr_index++);
#endif
#if SNSR_USE_ACCEL_Y
    written += snprintf(json_config_str+written, maxlen-written, "\"AccelerometerY\":%d,", snsr_index++);
#endif
#if SNSR_USE_ACCEL_Z
    written += snprintf(json_config_str+written, maxlen-written, "\"AccelerometerZ\":%d,", snsr_index++);
#endif
#if SNSR_USE_GYRO_X
    written += snprintf(json_config_str+written, maxlen-written, "\"GyroscopeX\":%d,", snsr_index++);
#endif
#if SNSR_USE_GYRO_Y
    written += snprintf(json_config_str+written, maxlen-written, "\"GyroscopeY\":%d,", snsr_index++);
#endif
#if SNSR_USE_GYRO_Z
    written += snprintf(json_config_str+written, maxlen-written, "\"GyroscopeZ\":%d", snsr_index++);
#endif
    if(json_config_str[written-1] == ',')
    {
        written--;
    }
    snprintf(json_config_str+written, maxlen-written, "}}\n");
    
    return written;
}

#endif //STREAM_FORMAT_IS(SMLSS)

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    int8_t app_failed;
#if STREAM_FORMAT_IS(SMLSS)
    uint32_t ssi_adtimer = 0;
    ssi_io_funcs_t ssi_io_s;
#endif
    
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    /* Register and start the LED ticker */
    TC_TimerCallbackRegister(Ticker_Callback);
    TC_TimerStart();
    
    /* Activate External Interrupt Controller for sensor capture */
    MIKRO_INT_CallbackRegister(SNSR_ISR_HANDLER);
    
    printf("\n");
    
    /* Application init routine */
    app_failed = 1;
    while (1)
    {
        /* Initialize our data buffer */
        buffer_init(&snsr_buffer);

        /* Initialize the UART RX buffer */
        if (!ringbuffer_init(&uartRxBuffer, _uartRxBuffer_data, sizeof(_uartRxBuffer_data)))
            break;
        
        /* Enable UART RXC interrupt */
        UART_RXC_Enable();

        if (sensor_init(&sensor) != SNSR_STATUS_OK) {
            printf("ERROR: sensor init result = %d\n", sensor.status);
            break;
        }
        
        if (sensor_set_config(&sensor) != SNSR_STATUS_OK) {
            printf("ERROR: sensor configuration result = %d\n", sensor.status);
            break;
        }
        
        printf("sensor type is %s\n", SNSR_NAME);
        printf("sensor sample rate set at %dHz\n", SNSR_SAMPLE_RATE_IN_HZ);
#if SNSR_USE_ACCEL
        printf("accelerometer axes %s%s%s enabled with range set at +/-%dGs\n", SNSR_USE_ACCEL_X ? "x" : "", SNSR_USE_ACCEL_Y ? "y" : "", SNSR_USE_ACCEL_Z ? "z" : "", SNSR_ACCEL_RANGE);
#else
        printf("accelerometer disabled\n");
#endif
#if SNSR_USE_GYRO
        printf("gyrometer axes %s%s%s enabled with range set at %dDPS\n", SNSR_USE_GYRO_X ? "x" : "", SNSR_USE_GYRO_Y ? "y" : "", SNSR_USE_GYRO_Z ? "z" : "", SNSR_GYRO_RANGE);
#else
        printf("gyrometer disabled\n");
#endif
        
#if STREAM_FORMAT_IS(SMLSS)
        /* Init SensiML simple-stream interface */
        ssi_io_s.ssi_read = UART_Read;
        ssi_io_s.ssi_write = UART_Write;
        ssi_io_s.connected = false;
        ssi_init(&ssi_io_s);
        ssi_build_json_config(json_config_str, SML_MAX_CONFIG_STRLEN);
#endif

        /* STATE CHANGE - Application successfully initialized */
        tickrate = 0; LED_STATUS_On();
        
#if STREAM_FORMAT_IS(SMLSS)
        /* STATE CHANGE - Application now waiting for connect */
#elif !STREAM_FORMAT_IS(NONE)
        /* STATE CHANGE - Application is streaming */
        tickrate = TICK_RATE_SLOW;
#endif //STREAM_FORMAT_IS(SMLSS)        
        
        buffer_reset(&snsr_buffer);
        app_failed = 0;
        break;
    }
    
    while (!app_failed)
    {
        /* Maintain state machines of all system modules. */
        SYS_Tasks ( );
        
        if (sensor.status != SNSR_STATUS_OK) {
            printf("ERROR: Got a bad sensor status: %d\n", sensor.status);
            break;
        }
#if STREAM_FORMAT_IS(SMLSS)
        else if (!ssi_connected()) {
            if (ringbuffer_get_read_bytes(&uartRxBuffer) >= CONNECT_CHARS) {
                ssi_try_connect();
            }
            if (ssi_connected()) {
                /* STATE CHANGE - Application is streaming */
                tickrate = TICK_RATE_SLOW;

                /* Reset the sensor buffer */
                buffer_reset(&snsr_buffer);
            }
            if (read_timer_ms() - ssi_adtimer > 500) {
                ssi_adtimer = read_timer_ms();
                UART_Write((uint8_t *) json_config_str, strlen(json_config_str));
            }
        }
#endif        
        else if (snsr_buffer.overrun == true) {
            printf("\n\n\nOverrun!\n\n\n");
            
            /* STATE CHANGE - buffer overflow */
            tickrate = 0;
            LED_ALL_Off();
            LED_STATUS_On(); LED_RED_On();  // Indicate OVERFLOW
            sleep_ms(5000U);
            
            // Clear OVERFLOW
            LED_ALL_Off();
            buffer_reset(&snsr_buffer); 
            
            /* STATE CHANGE - Application is streaming */
            tickrate = TICK_RATE_SLOW;
            continue;
        }
#if !STREAM_FORMAT_IS(NONE)
        else if(buffer_get_read_frames(&snsr_buffer) >= SNSR_SAMPLES_PER_PACKET) {            
            buffer_data_t *ptr;
            size_t rdcnt = buffer_get_read_buffer(&snsr_buffer, &ptr);
            while (rdcnt >= SNSR_SAMPLES_PER_PACKET) {
    #if STREAM_FORMAT_IS(ASCII)
                printf("%d", ptr[0]);
                for (int j=1; j < SNSR_NUM_AXES*SNSR_SAMPLES_PER_PACKET; j++) {
                    printf(" %d", ptr[j]);
                }
                printf("\n");
    #elif STREAM_FORMAT_IS(MDV)
                uint8_t headerbyte = MDV_START_OF_FRAME;
                UART_Write(&headerbyte, 1);
                UART_Write((uint8_t *) ptr, sizeof(buffer_frame_t)*SNSR_SAMPLES_PER_PACKET);
                headerbyte = ~headerbyte;
                UART_Write(&headerbyte, 1);
    #elif STREAM_FORMAT_IS(SMLSS)
                #if (SSI_JSON_CONFIG_VERSION == 2)
                ssiv2_publish_sensor_data(0, (uint8_t*) ptr, SNSR_SAMPLES_PER_PACKET*sizeof(buffer_frame_t));
                #elif (SSI_JSON_CONFIG_VERSION == 1)
                ssiv1_publish_sensor_data((uint8_t*) ptr, SNSR_SAMPLES_PER_PACKET*sizeof(buffer_frame_t));
                #endif
    #endif //STREAM_FORMAT_IS(ASCII)
                rdcnt -= SNSR_SAMPLES_PER_PACKET;
                ptr += SNSR_NUM_AXES * SNSR_SAMPLES_PER_PACKET;
                buffer_advance_read_index(&snsr_buffer, SNSR_SAMPLES_PER_PACKET);
            }
        }
#else   /* Template code for processing sensor data */        
        else {
            buffer_data_t *ptr;
            size_t rdcnt = buffer_get_read_buffer(&snsr_buffer, &ptr);
            while (rdcnt--) {
                // process sesnsor data
                buffer_advance_read_index(&snsr_buffer, 1);
            }
        }
#endif //!STREAM_FORMAT_IS(NONE)

#if STREAM_FORMAT_IS(SMLSS)        
        if (ssi_connected() && ringbuffer_get_read_bytes(&uartRxBuffer) >= DISCONNECT_CHARS) {
            ssi_try_disconnect();
            if (!ssi_connected()) {
                /* STATE CHANGE - Application now waiting for connect */
                tickrate = 0; LED_STATUS_On();
            }
        }
#endif
        
    }
        
    tickrate = 0;
    LED_STATUS_Off();
    LED_RED_On();
    
    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

