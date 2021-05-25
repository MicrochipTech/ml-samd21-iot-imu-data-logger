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
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "buffer.h"
#include "sensor.h"
#include "app_config.h"

#define SYSTICK_FREQ_IN_MHZ 48 // !NB! This must be changed if the processor clock changes

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

static volatile uint32_t tickcounter = 0;

static struct sensor_device_t sensor;
static struct sensor_buffer_t snsr_buffer;

void SYSTICK_Callback(uintptr_t context) {
    static int mstick = 0;
    int tickrate = *((int *) context);
    
    ++tickcounter;
    if (tickrate == 0) {
        mstick = 0;
    }
    else if (++mstick == tickrate) {
        LED_GREEN_Toggle();
        mstick = 0;
    }
}

uint64_t read_timer_ms(void) {
    return tickcounter;
}

uint64_t read_timer_us(void) {
    return tickcounter * 1000 + SYSTICK_TimerCounterGet() / SYSTICK_FREQ_IN_MHZ;
}

void sleep_ms(uint32_t ms) {
    SYSTICK_TimerStop();
    tickcounter = 0;
    SYSTICK_TimerStart();
    while (read_timer_ms() < ms) {};
}

void sleep_us(uint32_t us) {
    SYSTICK_TimerStop();
    tickcounter = 0;
    SYSTICK_TimerStart();
    while (read_timer_us() < us) {};
}


// For handling read of the sensor data
void SNSR_ISR_HANDLER(uintptr_t context) {
    struct sensor_device_t *sensor = (struct sensor_device_t *) context;
    
    /* Check if any errors we've flagged have been acknowledged */
    if (sensor->status != SNSR_STATUS_OK) {
        return;
    }
    
    sensor->status = sensor_read(sensor, &snsr_buffer);
}

int main ( void )
{
    int tickrate = 0;
    
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    /* Register and start the LED ticker */
    SYSTICK_TimerCallbackSet(SYSTICK_Callback, (uintptr_t) &tickrate);
    SYSTICK_TimerStart();
    
    /* Activate External Interrupt Controller for sensor capture */
    EIC_CallbackRegister(MIKRO_EIC_PIN, SNSR_ISR_HANDLER, (uintptr_t) &sensor);
    EIC_InterruptEnable(MIKRO_EIC_PIN);
       
    /* Initialize our data buffer */
    buffer_init(&snsr_buffer);
    
    printf("\r\n");
    
    while (1)
    {    
        if (sensor_init(&sensor) != SNSR_STATUS_OK) {
            printf("sensor init result = %d\r\n", sensor.status);
            break;
        }
        
        if (sensor_set_config(&sensor) != SNSR_STATUS_OK) {
            printf("sensor configuration result = %d\r\n", sensor.status);
            break;
        }
        
        printf("sensor sample rate set at %dHz\r\n", SNSR_SAMPLE_RATE);
        tickrate = TICK_RATE_SLOW;
        
        break;
    }
    
    while (1)
    {
        if (sensor.status != SNSR_STATUS_OK) {
            printf("Got a bad sensor status: %d\r\n", sensor.status);
            break;
        }
        else if (snsr_buffer.overrun == true) {
            printf("\r\n\r\n\r\nOverrun!\r\n\r\n\r\n");
            
            // Light the LEDs to indicate overflow
            tickrate = 0;
            LED_YELLOW_On();  // Indicate OVERFLOW
            sleep_ms(5000U);
            LED_YELLOW_Off(); // Clear OVERFLOW
            tickrate = TICK_RATE_SLOW;
            
            buffer_reset(&snsr_buffer);
            continue;
        }  
        else {
            // Feed temp buffer
            buffer_data_t *ptr;
            int rdcnt = buffer_get_read_buffer(&snsr_buffer, &ptr);

            while ( --rdcnt >= 0 ) {
#if DATA_VISUALIZER_BUILD
                uint8_t headerbyte = MPDV_START_OF_FRAME;
                
                SERCOM5_USART_Write(&headerbyte, 1);
                
                SERCOM5_USART_Write(ptr, SNSR_NUM_AXES*sizeof(buffer_data_t));
                
                headerbyte = ~headerbyte;
                SERCOM5_USART_Write(&headerbyte, 1);
                headerbyte = ~headerbyte;
#elif DATA_LOGGER_BUILD
                printf("%d", ptr[0]);
                for (int j=1; j < SNSR_NUM_AXES; j++) {
                    printf(" %d", ptr[j]);
                }
                printf("\r\n");
#endif
                ptr += SNSR_NUM_AXES;
                buffer_advance_read_index(&snsr_buffer, 1);
            }
        }
        
    }
    
    tickrate = 0;
    LED_GREEN_Off();
    LED_RED_On();
    
    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

