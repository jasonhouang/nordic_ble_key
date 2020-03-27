#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"
#include "ble_dtm.h"
#include "app_uart.h"
#include "app_timer.h"
#include "common.h"
#include "nrf_log.h"

#define MAX_ITERATIONS_NEEDED_FOR_NEXT_BYTE ((5000 + 2 * UART_POLL_CYCLE) / UART_POLL_CYCLE)

APP_TIMER_DEF(m_timer_dtm);

volatile bool is_msb_read = false; // True when MSB of the DTM command has been read and the application is waiting for LSB.
volatile uint16_t dtm_cmd_from_uart = 0; // Packed command containing command_code:freqency:length:payload in 2:6:6:2 bits.
bool is_dtm_mode_inited = false;

void dtm_timer_start(void)
{
    app_timer_start(m_timer_dtm, APP_TIMER_TICKS(5), NULL);
}

void dtm_timer_stop(void)
{
    app_timer_stop(m_timer_dtm);
}

static void app_timer_dtm_handle(void * p_context)
{
    is_msb_read = false;
    NRF_LOG_INFO("app_timer_dtm_handle");
}

/**@brief Function for splitting UART command bit fields into separate command parameters for the DTM library.
 *
 * @param[in]   command   The packed UART command.
 * @return      result status from dtmlib.
 */
static uint32_t dtm_cmd_put(uint16_t command)
{
    dtm_cmd_t      command_code = (command >> 14) & 0x03;
    dtm_freq_t     freq         = (command >> 8) & 0x3F;
    uint32_t       length       = (command >> 2) & 0x3F;
    dtm_pkt_type_t payload      = command & 0x03;

    return dtm_cmd(command_code, freq, length, payload);
}


/**@brief Function for application main entry.
 *
 * @details This function serves as an adaptation layer between a 2-wire UART interface and the
 *          dtmlib. After initialization, DTM commands submitted through the UART are forwarded to
 *          dtmlib and events (i.e. results from the command) is reported back through the UART.
 */
int dtm_main(void)
{
    uint32_t    current_time;
    uint32_t    dtm_error_code;
    uint32_t    msb_time          = 0;     // Time when MSB of the DTM command was read. Used to catch stray bytes from "misbehaving" testers.
    bool        is_msb_read       = false; // True when MSB of the DTM command has been read and the application is waiting for LSB.
    uint16_t    dtm_cmd_from_uart = 0;     // Packed command containing command_code:freqency:length:payload in 2:6:6:2 bits.
    uint8_t     rx_byte;                   // Last byte read from UART.
    dtm_event_t result;                    // Result of a DTM operation.

    printf("enter dtm mode\r\n");
    wdt_feed();
    app_timer_create(&m_timer_dtm, APP_TIMER_MODE_SINGLE_SHOT, app_timer_dtm_handle);

    dtm_error_code = dtm_init();           // Need to change default default timer0 in sdk
    if (dtm_error_code != DTM_SUCCESS)
    {
        // If DTM cannot be correctly initialized, then we just return.
        printf("DTM FUALT_%ld\r\n", dtm_error_code);
        return -1;
    }
    printf("dtm init success\r\n");

    is_dtm_mode_inited = true;

    for (;;)
    {
        wdt_feed();
        // Will return every timeout, 625 us.
        current_time = dtm_wait();

        (void)current_time;

        if (!is_msb_read)
        {
            continue;
        }

        if (dtm_cmd_put(dtm_cmd_from_uart) != DTM_SUCCESS)
        {
            // Extended error handling may be put here.
            // Default behavior is to return the event on the UART (see below);
            // the event report will reflect any lack of success.
        }

        // Retrieve result of the operation. This implementation will busy-loop
        // for the duration of the byte transmissions on the UART.
        if (dtm_event_get(&result))
        {
            // Report command status on the UART.
            // Transmit MSB of the result.
            while (app_uart_put((result >> 8) & 0xFF));
            // Transmit LSB of the result.
            while (app_uart_put(result & 0xFF));
            NRF_LOG_INFO("result = 0x%x", result);
        }

        is_msb_read = false;
    }
}

/// @}
