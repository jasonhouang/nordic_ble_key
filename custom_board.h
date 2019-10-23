#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions for CUSTOM_BOARD
#define LEDS_NUMBER    1

#define LED_START      12
#define LED_1          12
#define LED_STOP       12

#define LEDS_ACTIVE_STATE 0

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_LIST { LED_1 }

#define BSP_LED_0      LED_1

#define BUTTONS_NUMBER 2

#define BUTTON_START   14
#define BUTTON_1       14
#define BUTTON_2       15
#define BUTTON_STOP    15
#define BUTTON_PULL    NRF_GPIO_PIN_NOPULL

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2 }

#define BSP_BUTTON_0        BUTTON_1
#define BSP_BUTTON_1        BUTTON_2

#define BUTTON_CLOSE_PIN  BSP_BUTTON_0
#define BUTTON_OPEN_PIN   BSP_BUTTON_1

#define RX_PIN_NUMBER  18
#define TX_PIN_NUMBER  16
#define CTS_PIN_NUMBER 0
#define RTS_PIN_NUMBER 0
#define HWFC           false

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source       = NRF_CLOCK_LF_SRC_XTAL,      \
                                 .rc_ctiv      = 0,                          \
                                 .rc_temp_ctiv = 0,                          \
                                 .accuracy     = NRF_CLOCK_LF_ACCURACY_20_PPM}

#ifdef __cplusplus
}
#endif

#endif // PCA10040_H
