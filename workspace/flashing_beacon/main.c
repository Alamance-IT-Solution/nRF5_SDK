#include "bsp.h"
#include "boards.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_tps.h"


/**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_CONN_CFG_TAG            1

/**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
#define APP_COMPANY_IDENTIFIER          0x0059

/**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(100, UNIT_0_625_MS)

/**< Total length of information advertised by the Beacon. */
#define APP_BEACON_INFO_LENGTH          0x17
#define APP_ADV_DATA_LENGTH             0x15                               /**< Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE                 0x02                               /**< 0x02 refers to Beacon. */
#define APP_MEASURED_RSSI               0xC3                               /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
#define APP_COMPANY_IDENTIFIER          0x0059                             /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
#define APP_MAJOR_VALUE                 0x42, 0x42                         /**< Major value used to identify Beacons. */
#define APP_MINOR_VALUE                 0x03, 0x04                         /**< Minor value used to identify Beacons. */
#define APP_BEACON_UUID                 0x01, 0x12, 0x23, 0x34, \
                                        0x45, 0x56, 0x67, 0x78, \
                                        0x89, 0x9a, 0xab, 0xbc, \
                                        0xcd, 0xde, 0xef, 0xf0            /**< Proprietary UUID for Beacon. */

#define TX_POWER_LEVEL                  (4)                               /**< TX Power Level value. This will be set both in the TX Power service, in the advertising data, and also used to set the radio transmit power. */
// #define TX_POWER_LEVEL                  (-40)                               /**< TX Power Level value. This will be set both in the TX Power service, in the advertising data, and also used to set the radio transmit power. */


#define DEAD_BEEF                       0xDEADBEEF                         /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define ADVERTISE_DURATION              1000

#define MAJ_VAL_OFFSET_IN_BEACON_INFO   18                                 /**< Position of the MSB of the Major Value in m_beacon_info array. */
#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
#define UICR_ADDRESS                    0x10001080                         /**< Address of the UICR register used by this example. The major and minor versions to be encoded into the advertising data will be picked up from this location. */
#endif

BLE_TPS_DEF(m_tps);                                     /**< TX Power service instance. */

typedef enum {ALARM_0, ALARM_1, NUM_ALARMS} Alarm_t;

/**< Parameters to be passed to the stack when starting advertising. */
static ble_gap_adv_params_t m_adv_params;
/**< Advertising handle used to identify an advertising set. */
static uint8_t              m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
/**< Buffer for storing an encoded advertising set. */
static uint8_t              m_enc_advdata[NUM_ALARMS][BLE_GAP_ADV_SET_DATA_SIZE_MAX];


/**< Information advertised by the Beacon. */
static uint8_t m_beacon_info[APP_BEACON_INFO_LENGTH] =
{
    APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this
                         // implementation.
    APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the
                         // manufacturer specific data in this implementation.
    APP_BEACON_UUID,     // 128 bit UUID value.
    APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons.
    APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons.
    APP_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in
                         // this implementation.
};


/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data[NUM_ALARMS] = {
  { // ALARM_0
      .adv_data =
      {
          .p_data = m_enc_advdata[ALARM_0],
          .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
      },
      .scan_rsp_data =
      {
          .p_data = NULL,
          .len    = 0

      }
  },
  { // ALARM_1
      .adv_data =
      {
          .p_data = m_enc_advdata[ALARM_1],
          .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
      },
      .scan_rsp_data =
      {
          .p_data = NULL,
          .len    = 0

      }
  }
};


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


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for starting advertising.
 */
static void advertising_start(Alarm_t alarm)
{
    ret_code_t err_code;

    { // Build and set advertising data.
        ble_advdata_manuf_data_t manuf_specific_data;
        manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;
        manuf_specific_data.data.p_data = (uint8_t *) m_beacon_info;
        m_beacon_info[MAJ_VAL_OFFSET_IN_BEACON_INFO] = (uint8_t)alarm; // encode alarm type using APP_MAJOR_VALUE
        manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;

        ble_advdata_t advdata;
        memset(&advdata, 0, sizeof(advdata));
        advdata.name_type             = BLE_ADVDATA_NO_NAME;
        advdata.flags                 = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
        advdata.p_manuf_specific_data = &manuf_specific_data;

        err_code = ble_advdata_encode(&advdata, m_adv_data[alarm].adv_data.p_data, &m_adv_data[alarm].adv_data.len);
        APP_ERROR_CHECK(err_code);
    }

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data[alarm], NULL /* advertising parameters already set */);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}

/**
// Function to set up the BLE advertising data
static void setup_ble_advertisement_data(ble_advdata_t * p_advdata)
{
    // Set the advertising flags
    p_advdata->flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    // Set the device name
    p_advdata->name_type = BLE_ADVDATA_FULL_NAME;
    // Adjust the short_name_len field accordingly if using short name

    // Add additional data if needed, e.g., appearance, UUIDs, etc.
    p_advdata->include_appearance = true;
    ble_uuid_t uuid_array[1] = {{0x180D}};
    p_advdata->uuids_more_available.p_uuids = uuid_array;
    p_advdata->uuids_more_available.uuid_cnt = 1;

    // Call the advertising data encoding function
    ret_code_t err_code = ble_advdata_encode(p_advdata, NULL, NULL);
    if (err_code != NRF_SUCCESS)
    {
        printf("Error encoding advertising data. Error code: %u\n", (unsigned int)err_code);
    }
}
**/


/**
 * @brief Function for handling advertising events.
 *
 * @details This function is called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
/**
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = app_timer_start(m_blink_ble,
                                       APP_TIMER_TICKS(LED_BLINK_INTERVAL),
                                       (void *) LED_BLE_NUS_CONN);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            NRF_LOG_INFO("Advertising timeout, restarting.")
            advertising_start();
            break;
        default:
            break;
    }
}
**/


/**@brief Function for handling advertising events.

   @details This function will be called for advertising events which are passed to the application.

   @param[in] ble_adv_evt  Advertising event.
 */
 /**
void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            NRF_LOG_INFO("Advertising stopped.");
            break;

        default:
            break;
    }
}
**/

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void setup_ble_advertising_parameters(void)
{
    uint32_t err_code;

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval        = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.duration        = ADVERTISE_DURATION;       // Advertising duration in 10 ms units.

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, NULL /* no advertising data yet */, &m_adv_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the TX Power Service.
 */
static void tps_init(void)
{
    ret_code_t     err_code;
    ble_tps_init_t tps_init_obj;

    memset(&tps_init_obj, 0, sizeof(tps_init_obj));
    tps_init_obj.initial_tx_power_level = TX_POWER_LEVEL;

    tps_init_obj.tpl_rd_sec = SEC_JUST_WORKS;

    err_code = ble_tps_init(&m_tps, &tps_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    bsp_board_init(BSP_INIT_LEDS);
    ble_stack_init();
    setup_ble_advertising_parameters();
    tps_init();

    // Start execution.
    advertising_start(ALARM_0);

    while (true) {

    }

    /**
    ble_advdata_t advdata;
    ble_advdata_t scan_rsp_data;

    setup_ble_advertisement_data(&advdata);

    bsp_board_led_on(BSP_BOARD_LED_1);

    // Create a one element array containing a compound literal (object) with the UUID field set to 0x180F
    ble_uuid_t new_uuid[1] = {{.uuid = 0x180F}};
    // See: NRF5_SDK/components/softdevice/s122/headers/ble_types.h:ble_uuid_t
    // See: NRF5_SDK/components/ble/common/ble_advdata.h:ble_advdata_uuid_list_t
    // See: NRF5_SDK/components/ble/common/ble_advdata.h:ble_advdata_t

    while (true)
    {
        // Update the advertising data periodically (every 5 seconds in this example)
        ret_code_t err_code = ble_advertising_advdata_update(&advertising, &advdata, &scan_rsp_data);
        printf("Error code: %u\n", (unsigned int)err_code);
        simple_delay(25000);

        advdata.include_appearance = false;  // Appearance represents physical shape and role of the device
        new_uuid[0] = (ble_uuid_t){.uuid = 0x180F};  // Update the UUID value

        // A list of UUIDs that don't fit into the main advertising UUID
        advdata.uuids_more_available.p_uuids = new_uuid;

        // The number of UUIDs in the list
        advdata.uuids_more_available.uuid_cnt = 1;
        bsp_board_led_off(BSP_BOARD_LED_1);

        // Update the advertising data again
        err_code = ble_advertising_advdata_update(&advertising, &advdata, &scan_rsp_data);
        printf("Error code: %u\n", (unsigned int)err_code);
        simple_delay(25000);

        advdata.include_appearance = false;  // Appearance represents physical shape and role of the device
        new_uuid[0] = (ble_uuid_t){.uuid = 0x090E};  // Update the UUID value

        // A list of UUIDs that don't fit into the main advertising UUID
        advdata.uuids_more_available.p_uuids = new_uuid;
        bsp_board_led_on(BSP_BOARD_LED_1);

        // The number of UUIDs in the list
        advdata.uuids_more_available.uuid_cnt = 1;

    }
    **/
}

