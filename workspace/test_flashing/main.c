#include "nrf_sdh.h"
#include "boards.h"
#include "app_timer.h"
#include "app_button.h"


static void button_handler(uint8_t pin, uint8_t action) {
    if (pin == BSP_BUTTON_0) {
        if (action == APP_BUTTON_PUSH) {
            bsp_board_led_on(BSP_BOARD_LED_1);
        } else if (action == APP_BUTTON_RELEASE) {
            bsp_board_led_off(BSP_BOARD_LED_1);
        }
    }
}


/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    bsp_board_init(BSP_INIT_LEDS);
    app_timer_init();
    nrf_sdh_enable_request();
    static app_button_cfg_t buttons[] = {
    {
        BSP_BUTTON_0,
        false,
        BUTTON_PULL,
        button_handler
    }
    };

    app_button_init(buttons, ARRAY_SIZE(buttons), APP_TIMER_TICKS(50));
    app_button_enable();

}


/**
 * @}
 */
