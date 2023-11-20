#include "boards.h"


// Function for a simple busy-wait delay
void simple_delay(uint32_t milliseconds)
{
    // Assuming a 1 MHz system clock. Adjust the value accordingly.
    volatile uint32_t cycles = 1000 * milliseconds;

    while (cycles--)
    {
        // This loop will take approximately 1 second with a 1 MHz system clock
        // Adjust the loop duration based on your system clock frequency
        __NOP();
    }
}


/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    bsp_board_init(BSP_INIT_LEDS);

    // Our device is pca10059, not pca10056!
    while (true)
    {
        bsp_board_led_on(BSP_BOARD_LED_1);
        simple_delay(5000);
        bsp_board_led_off(BSP_BOARD_LED_1);
        simple_delay(5000);

    }
}

