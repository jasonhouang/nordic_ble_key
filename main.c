#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "app_scheduler.h"
#include "bsp_btn_ble.h"
#include "nrf_delay.h"
#include "nrf_drv_wdt.h"
#include "nrf_drv_power.h"
#include "sys_time.h"
#include "seed_manage.h"
#include "console.h"
#include "common.h"
#include "sm3.h"
#include "crc32.h"
#include "dtm.h"
#include "adc_user.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

#define NON_CONNECTABLE_ADV_INTERVAL    3200//MSEC_TO_UNITS(100, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define APP_BEACON_INFO_LENGTH          0x17                              /**< Total length of information advertised by the Beacon. */
#define APP_ADV_DATA_LENGTH             0x15                              /**< Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE                 0x02                              /**< 0x02 refers to Beacon. */
#define APP_MEASURED_RSSI               0xC3                              /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
#define APP_COMPANY_IDENTIFIER          0x0059                            /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
#define APP_MAJOR_VALUE                 0x01, 0x02                        /**< Major value used to identify Beacons. */
#define APP_MINOR_VALUE                 0x03, 0x04                        /**< Minor value used to identify Beacons. */

#define KEY_BEACON_UUID                 0xE7, 0xFC, 0x9D, 0x3C, \
                                        0xEF, 0x01, 0x4B, 0x70, \
                                        0xB2, 0x80, 0x2C, 0xF6, \
                                        0xD5, 0x0F, 0xA5, 0xCA            /**< UUID for BlE-Key: E7FC9D3C-EF01-4B70-B280-2CF6D50FA5CA */

#define BAT_BEACON_UUID                 0x8B, 0x91, 0xB3, 0xDF, \
                                        0xEF, 0x01, 0x42, 0xA1, \
                                        0xAF, 0x92, 0x25, 0xC2, \
                                        0x20, 0xA2, 0x4A, 0xAD            /**< UUID for LOW_BAT: 8B91B3DF-EF01-42A1-AF92-25C220A24AAD */

#define VOL_BEACON_UUID                 0xEC, 0xEF, 0xB9, 0x9D, \
                                        0xEF, 0x01, 0x4A, 0xCA, \
                                        0xAE, 0x4C, 0x61, 0xC8, \
                                        0xAB, 0xE1, 0x77, 0x50            /**< UUID for BAT_VOL: ECEFB99D-EF01-4ACA-AE4C-61C8ABE17750 */

#define DEVICE_NAME                     "Nordic_UART"                               /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      0                                         /**< The advertising timeout (in units of seconds). */

#define SCAN_INTERVAL                   150                                  /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                     150                                  /**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_TIMEOUT                    5                                    /**< Timout when scanning. 0x0000 disables timeout. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                512                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                128                                         /**< UART RX buffer size. */

#define TASK_TIMEOUT_INTERVAL           APP_TIMER_TICKS(1000)

#define APP_SCHED_MAX_EVENT_SIZE 1                  /**< Maximum size of scheduler events. */ 
#define APP_SCHED_QUEUE_SIZE     4                  /**< Maximum number of events in the scheduler queue. */

//#define printf(...)
#define CONSOLE_TIMEOUT                     60
#define VOLTAGE_CHECK_INTERVAL_NORMAL       3600
#define VOLTAGE_CHECK_INTERVAL_FAST         60

#define VOLTAGE_ADV_PERIOD                  60

enum {
    OPEN_LOCK = 0,
    CLOSE_LOCK,
};

typedef struct {
    uint8_t cmd;
} ctl_cmd_t;

BLE_NUS_DEF(m_nus);                                                                 /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */
APP_TIMER_DEF(m_task_timer_id);
APP_TIMER_DEF(m_timer_low_battery);

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};

static ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */

volatile bool m_scanner_started = false;
volatile bool m_adv_started = false;
static volatile uint16_t m_console_timeout = CONSOLE_TIMEOUT;
static uint32_t m_button_hold_timeout = BUTTON_HOLD_TIMEOUT;
static uint32_t m_voltage_check_interval = VOLTAGE_CHECK_INTERVAL_NORMAL;
static volatile uint32_t m_voltage_check_timeout = VOLTAGE_CHECK_INTERVAL_NORMAL;
static int32_t m_battery_voltage = 0;
static uint8_t vol_adv_count = VOLTAGE_ADV_PERIOD;
static bool is_advertising_changed = false;

#define TARGET_VENDOR_DATA_LEN              16
#define TARGET_PRODUCT_FILTER_DATA_LEN      6
#define TARGET_PRODUCT_FILTER_DATA          0x08, 0x00, 0x20, 0x0C, 0x9A, 0x66
const static uint8_t manufactuer_data[TARGET_VENDOR_DATA_LEN] = 
{
    0x11, 0xE8, 0xB5, 0x00,
    TARGET_PRODUCT_FILTER_DATA,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t key_crc24[7];
static uint8_t mac_set[] = {0x00, 0x01, 0xbe, 0xb9, 0x47, 0x81};
static ctl_cmd_t ctl_cmd = { .cmd = OPEN_LOCK };

typedef struct _ibeacon_t {
    uint8_t type;
    uint8_t length;
    uint8_t uuid[16];
    uint16_t major;
    uint16_t minor;
    uint8_t rssi;
} ibeacon_t;

static uint8_t m_beacon_info[APP_BEACON_INFO_LENGTH] =                    /**< Information advertised by the Beacon. */
{
    APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this
    // implementation.
    APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the
    // manufacturer specific data in this implementation.
    KEY_BEACON_UUID,     // 128 bit UUID value.
    APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons.
    APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons.
    APP_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in
        // this implementation.
};

static uint8_t m_beacon_bat[APP_BEACON_INFO_LENGTH] =                    /**< Information advertised by the Beacon. */
{
    APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this
    // implementation.
    APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the
    // manufacturer specific data in this implementation.
    BAT_BEACON_UUID,     // 128 bit UUID value.
    APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons.
    APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons.
    APP_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in
        // this implementation.
};

static uint8_t m_beacon_vol[APP_BEACON_INFO_LENGTH] =                    /**< Information advertised by the Beacon. */
{
    APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this
    // implementation.
    APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the
    // manufacturer specific data in this implementation.
    VOL_BEACON_UUID,     // 128 bit UUID value.
    APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons.
    APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons.
    APP_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in
        // this implementation.
};

static void local_mac_addr_set(const uint8_t * mac);
static void get_local_mac_addr(void);
static void scan_start(void);
static void scan_stop(void);

static void advertising_stop(void)
{
    ret_code_t err_code;

    if (m_adv_started)
    {
        ret_code_t err_code = sd_ble_gap_adv_stop();
        APP_ERROR_CHECK(err_code);

        m_adv_started = false;
    }
}
/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    ret_code_t err_code;

    if (!m_adv_started)
    {
        m_adv_started = true;

        err_code = sd_ble_gap_adv_start(&m_adv_params, APP_BLE_CONN_CFG_TAG);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        uint32_t err_code;

        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            do
            {
                err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                {
                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);
        }
        if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length-1] == '\r')
        {
            while (app_uart_put('\n') == NRF_ERROR_BUSY);
        }
    }

}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t       err_code;
    ble_nus_init_t nus_init;

    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
        default:
            break;
    }
}

static void parse_adv_report(ble_gap_evt_adv_report_t const * p_adv_report)
{
    ret_code_t   err_code;
    uint16_t     index  = 0;
    uint8_t    * p_data = (uint8_t *)p_adv_report->data;
    bool         is_target_product = false;
    bool         is_target_name = false;
    uint8_t    * p_ebox_state = NULL;
    uint8_t      device_name[10];

    while (index < p_adv_report->dlen)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];
        
        if (   (field_type == BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME)
            || (field_type == BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME ))
        {
            if ((field_length == 8) && (p_data[index + 2] == 'e'))
            {
                memcpy(device_name, &p_data[index + 2], field_length); 
                device_name[field_length - 1] = '\0';

                if (!memcmp(key_crc24, &p_data[index + 3], 6))
                {
                    is_target_name = true;
                }
            }
        }
        if (   (field_type == BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA)
            && (field_length == TARGET_VENDOR_DATA_LEN + 1))
        {
            if (!memcmp(&p_data[index + 6], &manufactuer_data[4], TARGET_PRODUCT_FILTER_DATA_LEN))
            {
                is_target_product = true;
                p_ebox_state = &p_data[index + 4];
            }
        }
        index += field_length + 1;
    }

    if (is_target_product && is_target_name)
    {
        if (p_ebox_state)
        {
            printf("scanned:%s, rssi = %d\r\n", device_name, p_adv_report->rssi);
            //printf("0x%02x,0x%02x\r\n", p_ebox_state[0], p_ebox_state[1]);
            if (p_ebox_state[1] == 0x68 && ctl_cmd.cmd == OPEN_LOCK)
            {
                if (get_key_state()->is_lock_state_changed)
                {
                    get_scan_count()->m_scan_count ++;
                    clear_lock_state_changed();
                }
                scan_stop();
            }
            else if (p_ebox_state[1] == 0x68 && ctl_cmd.cmd == CLOSE_LOCK)
            {
                set_lock_state_changed();
            }
            else if (p_ebox_state[1] == 0x69 && ctl_cmd.cmd == CLOSE_LOCK)
            {
                if (get_key_state()->is_lock_state_changed)
                {
                    get_scan_count()->m_scan_count ++;
                    clear_lock_state_changed();
                }
                scan_stop();
            }
            else if (p_ebox_state[1] == 0x69 && ctl_cmd.cmd == OPEN_LOCK)
            {
                set_lock_state_changed();
            }
        }
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (BLE_GAP_TIMEOUT_SRC_SCAN == p_ble_evt->evt.gap_evt.params.timeout.src)
            {
                clear_key_state_scan_open();
                clear_key_state_scan_close();
                m_scanner_started  = false;
                advertising_start();
                printf("scan timeout\r\n");
                get_scan_count()->m_scan_count ++;
            }
            break;
#ifndef S140
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;
#endif

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;
#if !defined (S112)
         case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
        {
            ble_gap_data_length_params_t dl_params;

            // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
            memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
            err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
            APP_ERROR_CHECK(err_code);
        } break;
#endif //!defined (S112)
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
        case BLE_GAP_EVT_ADV_REPORT:
        {
            ble_gap_evt_adv_report_t const * adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);

            parse_adv_report(adv_report);
        } break;
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
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

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, 64);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_KEY_0:
            printf("key0 \r\n");
            break;
        case BSP_EVENT_KEY_1:
            printf("key1 \r\n");
            break;
#if 0
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

#endif
        default:
            break;
    }
}


#if 0
/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;

            if ((data_array[index - 1] == '\n') || (data_array[index - 1] == '\r')  || (index >= (m_ble_nus_max_data_len)))
            {
                NRF_LOG_DEBUG("Ready to send data over BLE NUS");
                NRF_LOG_HEXDUMP_DEBUG(data_array, index);

                do
                {
                    uint16_t length = (uint16_t)index;
                    err_code = ble_nus_string_send(&m_nus, data_array, &length);
                    if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) )
                    {
                        APP_ERROR_CHECK(err_code);
                    }
                } while (err_code == NRF_ERROR_BUSY);

                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
#endif
/**@snippet [Handling the data received over UART] */


/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = 18,//RX_PIN_NUMBER,
        .tx_pin_no    = 16,//TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = NRF_UART_BAUDRATE_115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_HIGHEST,/*APP_IRQ_PRIORITY_LOWEST,*/
                       err_code);
    APP_ERROR_CHECK(err_code);
}
/**@snippet [UART Initialization] */


/**@brief Function for initializing the Advertising functionality.
 */
#if 0
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;//BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}
#else
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;

    ibeacon_t *p_beacon = (ibeacon_t *)&m_beacon_info;
    p_beacon->major = get_majorminor()->major;
    p_beacon->minor = get_majorminor()->minor;

    manuf_specific_data.data.p_data = (uint8_t *) m_beacon_info;
    manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
    m_adv_params.p_peer_addr = NULL;    // Undirected advertisement.
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.timeout     = 0;       // Never time out.
}

static void set_advertising_normal(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;

    ibeacon_t *p_beacon = (ibeacon_t *)&m_beacon_info;
    p_beacon->major = get_majorminor()->major;
    p_beacon->minor = get_majorminor()->minor;
    
    manuf_specific_data.data.p_data = (uint8_t *) m_beacon_info;
    manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);
}

static void set_advertising_battery(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;

    ibeacon_t *p_beacon = (ibeacon_t *)&m_beacon_bat;
    p_beacon->major = get_majorminor()->major;
    p_beacon->minor = get_majorminor()->minor;
    
    manuf_specific_data.data.p_data = (uint8_t *) m_beacon_bat;
    manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);
}

static void update_advertising_voltage(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;

    adc_user_get_vcc(&m_battery_voltage);
    uint8_t voltage_coded = m_battery_voltage / 10 - 100;
    ibeacon_t *p_beacon = (ibeacon_t *)&m_beacon_vol;

    p_beacon->major = 0x1000 | ((get_config()->beacon_id >> 8) & 0x0FFF);
    p_beacon->minor = (get_config()->beacon_id & 0x00FF) | voltage_coded;
    
    manuf_specific_data.data.p_data = (uint8_t *) m_beacon_vol;
    manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);
}
#endif

#if 1

const static uint8_t pcb_id[] = {0xA7, 0x2D, 0xFF, 0x8C, 0x96, 0x72}; // A72DFF8C9672
const static uint8_t key_seed[] = "13989DDD23516A7147DFF970F020684F0A5ECFC44D261123E1E64EC706578E7E";
//const static uint8_t pcb_id[] = {0x5a, 0xcf, 0x30, 0x2c, 0x34, 0x0d};
//const static uint8_t key_seed[] = "4C736A750EAFA28F6530532CB3561A4C56B2FE1B26397E1C668A25276FE8EB5A";

static void load_default_para(void)
{
    memcpy(get_config()->device_id, pcb_id, 6);

    if (!parse_seed_data((const char *)key_seed, get_config()->seed_data))
    {
        printf("parse_seed_data error\r\n");
    }

    uint8_t key_beacon_uuid[16] = {KEY_BEACON_UUID};
    memcpy(get_config()->uuid_normal, key_beacon_uuid, 16);

    uint8_t bat_beacon_uuid[16] = {BAT_BEACON_UUID};
    memcpy(get_config()->uuid_low_battery, bat_beacon_uuid, 16);

    uint8_t vol_beacon_uuid[16] = {VOL_BEACON_UUID};
    memcpy(get_config()->uuid_voltage, vol_beacon_uuid, 16);
}

void update_key_para(void)
{
    ibeacon_t * p_ibeacon = (ibeacon_t *)m_beacon_info;
    memcpy(p_ibeacon->uuid, get_config()->uuid_normal, UUID_LEN);

    p_ibeacon = (ibeacon_t *)m_beacon_bat;
    memcpy(p_ibeacon->uuid, get_config()->uuid_low_battery, UUID_LEN);

    p_ibeacon = (ibeacon_t *)m_beacon_vol;
    memcpy(p_ibeacon->uuid, get_config()->uuid_voltage, UUID_LEN);

    uint32_t pcb_id_crc = crc32_compute((uint8_t*)get_config()->device_id, 6, NULL);
    uint32_t key_crc = crc32_compute((uint8_t*)get_config()->seed_data, 32, &pcb_id_crc);

    get_config()->crc24 = key_crc & 0x00FFFFFF;
    //printf("crc24 = 0x%lx\r\n", get_config()->crc24);

    uint8_t crc24[3];
    crc24[0] = (uint8_t) ((get_config()->crc24 & 0x00FF0000) >> 16);
    crc24[1] = (uint8_t) ((get_config()->crc24 & 0x0000FF00) >> 8);
    crc24[2] = (uint8_t) ((get_config()->crc24 & 0x000000FF) >> 0);

    hex2str((const uint8_t *)crc24, 3, key_crc24);
    key_crc24[6] = '\0';

    //printf("key_crc24 = %s\r\n", key_crc24);
}

static void key_init(void)
{
    key_state_init();

    if (!load_config_from_flash())
    {
        printf("load config failed, use default\r\n");
        load_default_para();
    }

    update_key_para();
}

static uint8_t sum_check_gen(uint8_t mac4)
{
    uint8_t input[33];
    uint8_t output[4];
    uint8_t check = 0;

    input[0] = mac4;

    memcpy(&input[1], get_config()->seed_data, 32);

    sm3(input, 33, output);

    for (int i = 0; i < 4; i++)
    {
        check += output[i];
    }

    return check;
}

static void scheduler_scan_trigger(void * p_event_data, uint16_t event_size)
{
    //get_local_mac_addr();
    ctl_cmd = *(ctl_cmd_t *)p_event_data;

    if (ctl_cmd.cmd == OPEN_LOCK && get_key_state()->is_scan_open)
    {
        printf("scan open is started\r\n");
        return;
    }

    if (ctl_cmd.cmd == CLOSE_LOCK && get_key_state()->is_scan_close)
    {
        printf("scan close is started\r\n");
        return;
    }

    if (m_scanner_started)
    {
        uint32_t err_code = sd_ble_gap_scan_stop();
        APP_ERROR_CHECK(err_code);
        m_scanner_started = false;
    }
    else
    {
        advertising_stop();
    }

    mac_set[2] = (uint8_t)(get_config()->crc24);
    mac_set[3] = (uint8_t)(get_config()->crc24 >> 8);
    mac_set[4] = (uint8_t)(get_config()->crc24 >> 16);

    //printf("m_scan_count = %d\r\n", get_scan_count()->m_scan_count);
    if (ctl_cmd.cmd == OPEN_LOCK)
    {
        mac_set[1] = 0x01 | (get_scan_count()->m_scan_count << 1);
        mac_set[0] = sum_check_gen(mac_set[1]);
        local_mac_addr_set(mac_set);
        get_local_mac_addr();
        set_key_state_scan_open();
    }
    else
    {
        mac_set[1] = 0x00 | (get_scan_count()->m_scan_count << 1);
        mac_set[0] = sum_check_gen(mac_set[1]);
        local_mac_addr_set(mac_set);
        get_local_mac_addr();
        set_key_state_scan_close();
    }

    scan_start();
}
/**@brief Function for handling button events from app_button IRQ
 *
 * @param[in] pin_no        Pin of the button for which an event has occured
 * @param[in] button_action Press or Release
 */
static void button_evt_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

    if (pin_no == BUTTON_OPEN_PIN)
    {
        if (button_action == APP_BUTTON_PUSH)
        {
            //printf("open key push\r\n");
            set_key_state_bt_open_pushed();
            err_code = bsp_indication_set(BSP_INDICATE_SENT_OK);
            APP_ERROR_CHECK(err_code);

            ctl_cmd_t ctl_cmd = { .cmd = OPEN_LOCK };
            err_code = app_sched_event_put(&ctl_cmd, sizeof(ctl_cmd_t), scheduler_scan_trigger);
            APP_ERROR_CHECK(err_code);
        }
        else
        {
            set_key_state_bt_open_released();
            //printf("open key release\r\n");
        }
    }
    if (pin_no == BUTTON_CLOSE_PIN)
    {
        if (button_action == APP_BUTTON_PUSH)
        {
            set_key_state_bt_close_pushed();
            err_code = bsp_indication_set(BSP_INDICATE_SENT_OK);
            APP_ERROR_CHECK(err_code);
            //printf("close key push\r\n");
            ctl_cmd_t ctl_cmd = { .cmd = CLOSE_LOCK };
            err_code = app_sched_event_put(&ctl_cmd, sizeof(ctl_cmd_t), scheduler_scan_trigger);
            APP_ERROR_CHECK(err_code);
        }
        else
        {
            set_key_state_bt_close_released();
            //printf("close key release\r\n");
        }
    }
}

/**
 * @brief Function for initializing the registation button
 *
 * @retval Values returned by @ref app_button_init
 * @retval Values returned by @ref app_button_enable
 */
static void button_init(void)
{
    ret_code_t              err_code;
    const uint8_t           buttons_cnt  = 2;
    static app_button_cfg_t buttons_cfgs[2] = {
        {
            .pin_no         = BUTTON_OPEN_PIN,
            .active_state   = APP_BUTTON_ACTIVE_LOW,
            .pull_cfg       = NRF_GPIO_PIN_NOPULL,//UP,
            .button_handler = button_evt_handler
        },
        {
            .pin_no         = BUTTON_CLOSE_PIN,
            .active_state   = APP_BUTTON_ACTIVE_LOW,
            .pull_cfg       = NRF_GPIO_PIN_NOPULL,//UP,
            .button_handler = button_evt_handler
        }};

    err_code = app_button_init(buttons_cfgs, buttons_cnt, APP_TIMER_TICKS(10));
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}

static void buttons_leds_init(bool * p_erase_bonds)
{
    uint32_t err_code = bsp_init(BSP_INIT_LED, NULL);
    APP_ERROR_CHECK(err_code);

    button_init();
}
#endif

#if 0
/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}
#endif

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(get_beijing_time_ms);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static uint8_t voltage_endure_counter = 5;
static void check_battery(void)
{
    if (!get_key_state()->is_low_battery)
    {
        adc_user_get_vcc(&m_battery_voltage);
        printf("VCC: %ldmV\r\n", m_battery_voltage);
        if (m_battery_voltage < LOW_BATTERY)
        {
            m_voltage_check_interval = VOLTAGE_CHECK_INTERVAL_FAST;
            if (m_voltage_check_timeout > m_voltage_check_interval)
            {
                m_voltage_check_timeout = m_voltage_check_interval;
            }

            if (voltage_endure_counter)
            {
                voltage_endure_counter --;
            }
            else
            {
                set_key_state_low_battery();
                ret_code_t err_code = app_timer_start(m_timer_low_battery, APP_TIMER_TICKS(50), NULL);
                APP_ERROR_CHECK(err_code);
            }
        }
        else
        {
            m_voltage_check_interval = VOLTAGE_CHECK_INTERVAL_NORMAL;
            voltage_endure_counter = 5;
        }
    }
}


static void app_task_handler(void * p_context)
{
    //NRF_LOG_INFO("app_task_handler");
    //printf("%s\r\n", (char *)get_date_time());
    check_is_need_update_majorminor();

    if (vol_adv_count)
    {
        vol_adv_count --;
        if (!vol_adv_count)
        {
            is_advertising_changed = true;
            vol_adv_count = VOLTAGE_ADV_PERIOD;
            advertising_stop();
            update_advertising_voltage();
            advertising_start();
        }
    }
    if (is_advertising_changed)
    {
        is_advertising_changed = false;
        advertising_stop();
        if (get_key_state()->is_low_battery)
        {
            set_advertising_battery();
        }
        else
        {
            set_advertising_normal();
        }
        advertising_start();
    }

    if (m_console_timeout)
    {
        m_console_timeout --;
        if (!m_console_timeout)
        {
            printf("ZZzz...\r\n");
            uint32_t err_code = sd_app_evt_wait();
            APP_ERROR_CHECK(err_code);
            nrf_delay_ms(10);
            err_code = app_uart_close();
            APP_ERROR_CHECK(err_code);
            set_key_state_low_power();
        }
    }

    if (m_voltage_check_timeout)
    {
        m_voltage_check_timeout --;
        if (!m_voltage_check_timeout)
        {
            check_battery();        
            m_voltage_check_timeout = m_voltage_check_interval;
        }
    }

    if (get_key_state()->is_button_open_pushed && get_key_state()->is_button_close_pushed)
    {
        if (m_button_hold_timeout)
        {
            m_button_hold_timeout --;
            if (!m_button_hold_timeout)
            {
                uart_init();
                clear_key_state_low_power();
                m_console_timeout = CONSOLE_TIMEOUT;
                printf("Console avaliable in %d seconds.\r\n", CONSOLE_TIMEOUT);
            }
        }
    }
    else
    {
        m_button_hold_timeout = BUTTON_HOLD_TIMEOUT;
    }
}

static uint8_t low_battery_ticks = 0;
static void app_low_battery_handler(void * p_context)
{
    if (low_battery_ticks >= 200)
    {
        low_battery_ticks = 0;
    }
    if (low_battery_ticks < 10)
    {
        if (0 == (low_battery_ticks % 4))
        {
            bsp_board_led_on(BSP_LED_0);
        }
        else
        {
            bsp_board_led_off(BSP_LED_0);
        }
    }
    else
    {
        bsp_board_led_off(BSP_LED_0);
    }
    low_battery_ticks ++;
}

static void app_timer_scanner(void * p_context)
{
    ret_code_t err_code;
    printf("app_timer_scanner stopped\r\n");

    scan_stop();
}

/**@brief Function for initializing the timer module.
  */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_task_timer_id, APP_TIMER_MODE_REPEATED, app_task_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_timer_low_battery, APP_TIMER_MODE_REPEATED, app_low_battery_handler);
    APP_ERROR_CHECK(err_code);
}

static void application_timers_start(void)
{
     ret_code_t err_code;

     err_code = app_timer_start(m_task_timer_id, TASK_TIMEOUT_INTERVAL, NULL);
     APP_ERROR_CHECK(err_code);
}
/**@brief Function for placing the application in low power state while waiting for events.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

static ble_gap_irk_t set_irk = {{0x04, 0xF3, 0x29, 0x46, 0x84, 0x3F, 0x32, 0x8A, 0xCF, 0xAA, 0x85, 0x26, 0xEC, 0x50, 0x6C, 0x2D}};

static void privacy_on(void)
{
    ret_code_t               err_code;
    ble_gap_privacy_params_t privacy_params;

    // Privacy settings cannot be changed while advertising, scanning, or creating a connection.
    //scan_stop();
    //adv_stop();
    //err_code = sd_ble_gap_connect_cancel();
    //APP_ERROR_CHECK(err_code);

    privacy_params.p_device_irk         = NULL;
    err_code = sd_ble_gap_privacy_get(&privacy_params);
    APP_ERROR_CHECK(err_code);
    printf("privacy_params.privacy_mode = %x\r\n", privacy_params.privacy_mode);
    printf("privacy_params.private_addr_type = %x\r\n", privacy_params.private_addr_type);
    printf("privacy_params.private_addr_cycle_s = %x\r\n", privacy_params.private_addr_cycle_s);
    if (privacy_params.p_device_irk)
    {
        printf("privacy_params.p_device_irk = 0x%ln\r\n", (uint32_t *)&privacy_params.p_device_irk->irk[0]);
    }

    memset(&privacy_params, 0, sizeof(privacy_params));

    // Privacy setting.
    privacy_params.privacy_mode         = BLE_GAP_PRIVACY_MODE_DEVICE_PRIVACY;
    privacy_params.private_addr_type    = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE;
    privacy_params.private_addr_cycle_s = 60;//BLE_GAP_DEFAULT_PRIVATE_ADDR_CYCLE_INTERVAL_S;
    privacy_params.p_device_irk         = &set_irk;

    // Set privacy.
    err_code = sd_ble_gap_privacy_set(&privacy_params);
    APP_ERROR_CHECK(err_code);

    // Set device indentities list.
    err_code = sd_ble_gap_device_identities_set(NULL, NULL, 1);//NRF_SDH_BLE_PERIPHERAL_LINK_COUNT);
    APP_ERROR_CHECK(err_code);

    privacy_params.p_device_irk         = NULL;
    err_code = sd_ble_gap_privacy_get(&privacy_params);
    APP_ERROR_CHECK(err_code);
    printf("privacy_params.privacy_mode = %x\r\n", privacy_params.privacy_mode);
    printf("privacy_params.private_addr_type = %x\r\n", privacy_params.private_addr_type);
    printf("privacy_params.private_addr_cycle_s = %x\r\n", privacy_params.private_addr_cycle_s);
    if (privacy_params.p_device_irk)
    {
        printf("privacy_params.p_device_irk = 0x%ln\r\n", (uint32_t *)&privacy_params.p_device_irk->irk[0]);
    }
}

static void local_mac_addr_set(const uint8_t * mac)
{
    ret_code_t ret;

    ble_gap_addr_t ble_gap_addr = {
        .addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
        .addr = {0x00, 0x00, 0xbe, 0xb9, 0x47, 0x81}
    };

    if (mac)
    {
        memcpy(ble_gap_addr.addr, mac, BLE_GAP_ADDR_LEN);
    }

    ret = sd_ble_gap_addr_set(&ble_gap_addr);
    APP_ERROR_CHECK(ret);
}

static void get_local_mac_addr(void)
{
    ble_gap_addr_t own_addr;

    sd_ble_gap_addr_get(&own_addr);
    printf("type:%x mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n", own_addr.addr_type,
            own_addr.addr[5],own_addr.addr[4],
            own_addr.addr[3],own_addr.addr[2],
            own_addr.addr[1],own_addr.addr[0]);
}

/** @brief Parameters used when scanning. */
static ble_gap_scan_params_t const m_scan_params =
{
    .active   = 1,
    .interval = SCAN_INTERVAL,
    .window   = SCAN_WINDOW,
    .timeout  = SCAN_TIMEOUT,
    .use_whitelist = 0,
};

/**@brief Function to start scanning. */
static void scan_start(void)
{
    ret_code_t err_code;

    if (m_scanner_started)
    {
        printf("already scanning\r\n");
        return;
    }

    printf("start scan...\r\n");

    err_code = sd_ble_gap_tx_power_set(4);
    APP_ERROR_CHECK(err_code);
    
    m_scanner_started  = true;

    err_code = sd_ble_gap_scan_start(&m_scan_params);
    APP_ERROR_CHECK(err_code);
}

static void scan_stop(void)
{
    ret_code_t err_code;

    if (m_scanner_started)
    {
        err_code = sd_ble_gap_scan_stop();
        APP_ERROR_CHECK(err_code);

        clear_key_state_scan_open();
        clear_key_state_scan_close();

        m_scanner_started  = false;

        printf("scan stopped\r\n");

        advertising_start();
    }
}

static void wdt_event_handler(void)
{
}

nrf_drv_wdt_channel_id m_channel_id;

void wdt_feed(void)
{ 
    nrf_drv_wdt_channel_feed(m_channel_id);
}

static void wdt_init(void)
{
    uint32_t err_code;

    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);

    nrf_drv_wdt_enable();
}

static void log_resetreason(void)
{
    /* Reset reason */
    uint32_t rr = nrf_power_resetreas_get();
    NRF_LOG_INFO("Reset reasons:");
    printf("Reset reasons: ");
    if (0 == rr)
    {
        NRF_LOG_INFO("- NONE");
        printf("- NONE\r\n");
    }
    if (0 != (rr & NRF_POWER_RESETREAS_RESETPIN_MASK))
    {
        NRF_LOG_INFO("- RESETPIN");
        printf("- RESETPIN\r\n");
    }
    if (0 != (rr & NRF_POWER_RESETREAS_DOG_MASK     ))
    {
        NRF_LOG_INFO("- DOG");
        printf("- DOG\r\n");
    }
    if (0 != (rr & NRF_POWER_RESETREAS_SREQ_MASK    ))
    {
        NRF_LOG_INFO("- SREQ");
        printf("- SREQ\r\n");
    }
    if (0 != (rr & NRF_POWER_RESETREAS_LOCKUP_MASK  ))
    {
        NRF_LOG_INFO("- LOCKUP");
        printf("- LOCKUP\r\n");
    }
    if (0 != (rr & NRF_POWER_RESETREAS_OFF_MASK     ))
    {
        NRF_LOG_INFO("- OFF");
        printf("- OFF\r\n");
    }
    if (0 != (rr & NRF_POWER_RESETREAS_DIF_MASK     ))
    {
        NRF_LOG_INFO("- DIF");
        printf("- DIF\r\n");
    }

    nrf_power_resetreas_clear(nrf_power_resetreas_get());
}

static void enable_dcdc_mode(void)
{
    uint32_t err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);

    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_INFO("Function: %s, error code: %s", (uint32_t)__func__, 
                (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
    }

    APP_ERROR_CHECK(err_code);
}


/**@brief Application main function.
 */
int main(void)
{
    uint32_t err_code;
    bool     erase_bonds;

    timers_init();

    uart_init();
    log_init();
    wdt_init();
    init_sys_time();

    buttons_leds_init(&erase_bonds);
    APP_SCHED_INIT(APP_SCHED_MAX_EVENT_SIZE, APP_SCHED_QUEUE_SIZE);
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    log_resetreason();

    printf("\r\n[PHYSICAL KEY] Ver:%d.%d, Re:%s\r\n", VERSION, REVISON, __DATE__);
    key_init();
    enable_dcdc_mode();
    //privacy_on();
    local_mac_addr_set(mac_set);
    get_local_mac_addr();
    NRF_LOG_INFO("Application Start!");
    application_timers_start();
    advertising_start();
    check_battery();
    printf("%s\r\n", (char *)get_date_time());

    for (;;)
    {
        wdt_feed();
        UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
        app_sched_execute();
        power_manage();
        if (get_key_state()->is_dtm_mode)
        {
            err_code = app_timer_stop(m_task_timer_id);
            APP_ERROR_CHECK(err_code);
            break;
        }
    }

    dtm_main();
}


/**
 * @}
 */
