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
#define UART_RXBUF_LEN  128
static uint8_t _uartRxBuffer_data[UART_RXBUF_LEN];
static ringbuffer_t uartRxBuffer;

static volatile uint32_t tickcounter = 0;
static volatile unsigned int tickrate = 0;

static struct sensor_device_t sensor;
static snsr_data_t _snsr_buffer_data[SNSR_BUF_LEN][SNSR_NUM_AXES];
static ringbuffer_t snsr_buffer;
static volatile bool snsr_buffer_overrun = false;

// *****************************************************************************
// *****************************************************************************
// Section: Platform specific stub definitions
// *****************************************************************************
// *****************************************************************************
void SERCOM5_Handler() {
    ringbuffer_size_t rdcnt;
    uint8_t *ptr = ringbuffer_get_write_buffer(&uartRxBuffer, &rdcnt);
    if (UART_IsRxReady() && rdcnt) {
        *ptr = UART_RX_DATA;
        ringbuffer_advance_write_index(&uartRxBuffer, 1);
    }
}

size_t __attribute__(( unused )) UART_Write(uint8_t *ptr, const size_t nbytes) {
    return SERCOM5_USART_Write(ptr, nbytes) ? nbytes : 0;
}

// *****************************************************************************
// *****************************************************************************
// Section: Generic stub definitions
// *****************************************************************************
// *****************************************************************************
void Null_Handler() {
    // Do nothing
}

size_t __attribute__(( unused )) UART_Read(uint8_t *ptr, const size_t nbytes) {
    return ringbuffer_read(&uartRxBuffer, ptr, nbytes);
}

static void Ticker_Callback() {
    static uint32_t mstick = 0;

    ++tickcounter;
    if (tickrate == 0 || mstick > tickrate) {
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
    if ((sensor.status != SNSR_STATUS_OK) || snsr_buffer_overrun)
        return;
    
    ringbuffer_size_t wrcnt;
    snsr_data_t *ptr = ringbuffer_get_write_buffer(&snsr_buffer, &wrcnt);
    
    if (wrcnt == 0)
        snsr_buffer_overrun = true;
    else if ((sensor.status = sensor_read(&sensor, ptr)) == SNSR_STATUS_OK)
        ringbuffer_advance_write_index(&snsr_buffer, 1);
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
            , SSI_JSON_CONFIG_VERSION, SNSR_SAMPLE_RATE, SNSR_SAMPLES_PER_PACKET);
#if SNSR_USE_ACCEL
    written += snprintf(json_config_str+written, maxlen-written, "\"AccelerometerX\":%d,", snsr_index++);
    written += snprintf(json_config_str+written, maxlen-written, "\"AccelerometerY\":%d,", snsr_index++);
    written += snprintf(json_config_str+written, maxlen-written, "\"AccelerometerZ\":%d,", snsr_index++);
#endif
#if SNSR_USE_GYRO
    written += snprintf(json_config_str+written, maxlen-written, "\"GyroscopeX\":%d,", snsr_index++);
    written += snprintf(json_config_str+written, maxlen-written, "\"GyroscopeY\":%d,", snsr_index++);
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
    int8_t app_failed = 0;
#if STREAM_FORMAT_IS(SMLSS)
    uint32_t ssi_adtimer = 0;
    ssi_io_funcs_t ssi_io_s;
#endif

    /* Initialize all modules */
    SYS_Initialize ( NULL );

    /* Register and start the millisecond interrupt ticker */
    TC_TimerCallbackRegister(Ticker_Callback);
    TC_TimerStart();

    printf("\n");

    /* Application init routine */
    app_failed = 1;
    while (1)
    {
        /* Initialize the sensor data buffer */
        if (ringbuffer_init(&snsr_buffer, _snsr_buffer_data, sizeof(_snsr_buffer_data) / sizeof(_snsr_buffer_data[0]), sizeof(_snsr_buffer_data[0])))
            break;
    
        /* Initialize the UART RX buffer */
        if (ringbuffer_init(&uartRxBuffer, _uartRxBuffer_data, sizeof(_uartRxBuffer_data) / sizeof(_uartRxBuffer_data[0]), sizeof(_uartRxBuffer_data[0])))
            break;

        /* Enable the RX interrupt */
        UART_RXC_Enable();

        /* Init and configure sensor */
        if (sensor_init(&sensor) != SNSR_STATUS_OK) {
            printf("ERROR: sensor init result = %d\n", sensor.status);
            break;
        }

        if (sensor_set_config(&sensor) != SNSR_STATUS_OK) {
            printf("ERROR: sensor configuration result = %d\n", sensor.status);
            break;
        }

        printf("sensor type is %s\n", SNSR_NAME);
        printf("sensor sample rate set at %dHz\n", SNSR_SAMPLE_RATE);
#if SNSR_USE_ACCEL
        printf("accelerometer enabled with range set at +/-%dGs\n", SNSR_ACCEL_RANGE);
#else
        printf("accelerometer disabled\n");
#endif
#if SNSR_USE_GYRO
        printf("gyrometer enabled with range set at %dDPS\n", SNSR_GYRO_RANGE);
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

        /* Activate External Interrupt Controller for sensor capture */
        MIKRO_INT_CallbackRegister(SNSR_ISR_HANDLER);

        /* STATE CHANGE - Application successfully initialized */
        tickrate = 0;
        LED_ALL_Off();
        LED_STATUS_On();

#if STREAM_FORMAT_IS(SMLSS)
        /* STATE CHANGE - Application now waiting for connect */
#elif !STREAM_FORMAT_IS(NONE)
        /* STATE CHANGE - Application is streaming */
        tickrate = TICK_RATE_SLOW;
#endif //STREAM_FORMAT_IS(SMLSS)

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
            if (ringbuffer_get_read_items(&uartRxBuffer) >= CONNECT_CHARS) {
                ssi_try_connect();
                ringbuffer_advance_read_index(&uartRxBuffer, ringbuffer_get_read_items(&uartRxBuffer));
            }
            if (ssi_connected()) {
                /* STATE CHANGE - Application is streaming */
                tickrate = TICK_RATE_SLOW;

                /* Reset the sensor buffer */
                MIKRO_INT_CallbackRegister(Null_Handler);
                ringbuffer_reset(&snsr_buffer);
                snsr_buffer_overrun = false;
                MIKRO_INT_CallbackRegister(SNSR_ISR_HANDLER);
            }
            if (read_timer_ms() - ssi_adtimer > 500) {
                ssi_adtimer = read_timer_ms();
                UART_Write((uint8_t *) json_config_str, strlen(json_config_str));
            }
        }
#endif
        else if (snsr_buffer_overrun == true) {
            printf("\n\n\nOverrun!\n\n\n");

            /* STATE CHANGE - buffer overflow */
            tickrate = 0;
            LED_ALL_Off();
            LED_STATUS_On(); LED_RED_On();
            sleep_ms(5000U);

            // Clear OVERFLOW
            MIKRO_INT_CallbackRegister(Null_Handler);
            ringbuffer_reset(&snsr_buffer);
            snsr_buffer_overrun = false;
            MIKRO_INT_CallbackRegister(SNSR_ISR_HANDLER);

            /* STATE CHANGE - Application is streaming */
            tickrate = TICK_RATE_SLOW;
            LED_ALL_Off();
            continue;
        }
#if !STREAM_FORMAT_IS(NONE)
        else if(ringbuffer_get_read_items(&snsr_buffer) >= SNSR_SAMPLES_PER_PACKET) {
            ringbuffer_size_t rdcnt;
            snsr_dataframe_t const *ptr = ringbuffer_get_read_buffer(&snsr_buffer, &rdcnt);
            while (rdcnt >= SNSR_SAMPLES_PER_PACKET) {
    #if STREAM_FORMAT_IS(ASCII)
                snsr_data_t const *scalarptr = (snsr_data_t const *) ptr;
                printf("%d", *scalarptr++);
                for (int j=1; j < sizeof(snsr_datapacket_t) / sizeof(snsr_data_t); j++) {
                    printf(" %d", *scalarptr++);
                }
                printf("\n");
    #elif STREAM_FORMAT_IS(MDV)
                uint8_t headerbyte = MDV_START_OF_FRAME;
                UART_Write(&headerbyte, 1);
                UART_Write((uint8_t *) ptr, sizeof(snsr_datapacket_t));
                headerbyte = ~headerbyte;
                UART_Write(&headerbyte, 1);
    #elif STREAM_FORMAT_IS(SMLSS)
                #if (SSI_JSON_CONFIG_VERSION == 2)
                ssiv2_publish_sensor_data(0, (uint8_t*) ptr, sizeof(snsr_datapacket_t));
                #elif (SSI_JSON_CONFIG_VERSION == 1)
                ssiv1_publish_sensor_data((uint8_t*) ptr, sizeof(snsr_datapacket_t));
                #endif
    #endif //STREAM_FORMAT_IS(ASCII)
                ptr += SNSR_SAMPLES_PER_PACKET;
                rdcnt -= SNSR_SAMPLES_PER_PACKET;
                ringbuffer_advance_read_index(&snsr_buffer, SNSR_SAMPLES_PER_PACKET);
            }
        }
#else   /* Template code for processing sensor data */
        else {
            ringbuffer_size_t rdcnt;
            snsr_dataframe_t const *ptr = ringbuffer_get_read_buffer(&snsr_buffer, &rdcnt);
            while (rdcnt--) {
                // process sensor data
                ptr++;
                ringbuffer_advance_read_index(&snsr_buffer, 1);
            }
        }
#endif //!STREAM_FORMAT_IS(NONE)

#if STREAM_FORMAT_IS(SMLSS)
        if (ssi_connected() && ringbuffer_get_read_items(&uartRxBuffer) >= DISCONNECT_CHARS) {
            ssi_try_disconnect();
            ringbuffer_advance_read_index(&uartRxBuffer, ringbuffer_get_read_items(&uartRxBuffer));
            if (!ssi_connected()) {
                /* STATE CHANGE - Application now waiting for connect */
                tickrate = 0;
                LED_ALL_Off();
                LED_STATUS_On();
            }
        }
#endif

    }

    tickrate = 0;
    LED_ALL_Off();
    LED_RED_On();
    
    /* Loop forever on error */
    while (1) {};

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/
