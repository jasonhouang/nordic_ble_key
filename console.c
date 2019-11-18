#include <stdint.h>
#include <stdlib.h>
#include "nrf.h"
#include "nrf_sdm.h"
#include "nrf_mbr.h"
#include "app_uart.h"
#include "nrf_log.h"
#include "console.h"
#include "common.h"
#include "sys_time.h"

#define UART_RX_MAX_DATA_LEN                128
#define PARAM_SIZE                          UART_RX_MAX_DATA_LEN - 3

#define CMD_NN  (((uint16_t)'N'<<8)|'N') //Null command
#define CMD_DT  (((uint16_t)'D'<<8)|'T') //Go to DTM mode
#define CMD_RR  (((uint16_t)'R'<<8)|'R') //Launch a software reset
#define CMD_HI  (((uint16_t)'H'<<8)|'I') //Get MCU informations
#define CMD_SI  (((uint16_t)'S'<<8)|'I') //Get firmware informations
#define CMD_SN  (((uint16_t)'S'<<8)|'N') //Write and Read DEVICE ID
#define CMD_RV  (((uint16_t)'R'<<8)|'V') //Read VCC         
#define CMD_ST  (((uint16_t)'S'<<8)|'T') //RUN Self test    
#define CMD_DA  (((uint16_t)'D'<<8)|'A') //Set datetime
#define CMD_ED  (((uint16_t)'E'<<8)|'D') //Write and Read Beacon Seed       
#define CMD_SL  (((uint16_t)'S'<<8)|'L') //Sleep to low power mode, console is disabled.
#define CMD_UD  (((uint16_t)'U'<<8)|'D') //Set UUID normal
#define CMD_UL  (((uint16_t)'U'<<8)|'L') //Set UUID low battery

static uint8_t cmd_buffer[UART_RX_MAX_DATA_LEN];
static int8_t  index = 0;
static char    parameter[PARAM_SIZE+1];

static void outputDeviceInfo(void)
{
    char deviceName[16];
    char packageName[16];
    char flashInfo[16];
    char ramInfo[16];
    char Variant[5];
    sprintf(deviceName, "nRF%lX", NRF_FICR->INFO.PART);
    switch(NRF_FICR->INFO.PACKAGE)
    {
        case 0x2000:
            sprintf(packageName, "QFN48");
            break;
        case 0x2003:
            sprintf(packageName, "QFN32");
            break;
        case 0x2001:
            sprintf(packageName, "WLCSP(CH)");
            break;
        case 0x2002:
            sprintf(packageName, "WLCSP(CI)");
            break;
        case 0x2005:
            sprintf(packageName, "WLCSP(CK)");
            break;
        default:
            sprintf(packageName, "N/A");
            break;
    }

    switch(NRF_FICR->INFO.FLASH)
    {
        case 0x80:
            sprintf(flashInfo, "128kB");
            break;
        case 0xC0:
            sprintf(flashInfo, "192kB");
            break;
        case 0x100:
            sprintf(flashInfo, "256kB");
            break;
        case 0x200:
            sprintf(flashInfo, "512kB");
            break;

        default:
            sprintf(flashInfo, "N/A");
            break;
    }

    switch(NRF_FICR->INFO.RAM)
    {
        case 0x10:
            sprintf(ramInfo, "16kB");
            break;
        case 0x18:
            sprintf(ramInfo, "24kB");
            break;
        case 0x20:
            sprintf(ramInfo, "32kB");
            break;
        case 0x40:
            sprintf(ramInfo, "64kB");
            break;
        default:
            sprintf(ramInfo, "N/A");
            break;
    }
    if(NRF_FICR->INFO.VARIANT != 0xFFFFFFFF)
    {
        Variant[0] = NRF_FICR->INFO.VARIANT>>24;
        Variant[1] = NRF_FICR->INFO.VARIANT>>16;
        Variant[2] = NRF_FICR->INFO.VARIANT>>8;
        Variant[3] = NRF_FICR->INFO.VARIANT>>0;
        Variant[4] = 0;
    }
    else
    {
        sprintf(Variant, "N/A");
    }
    printf("MCU:%s,VAR:%s,PAK:%s,ROM:%s,RAM:%s\r\n",deviceName, Variant, packageName, flashInfo, ramInfo);
}

void console_process(void)
{
    uint8_t len, paramlen = 0;
    uint16_t cmdCode;
    uint64_t i;
    uint64_t tmp_64;
    sysTime_t current;
    char* p;
    config_t* config = get_config();

    len = strlen((char*)cmd_buffer);
    if (len >= 3)
    {
        if (cmd_buffer[0] == '$')
        {
            if(len > 3)
            {
                paramlen = len - 3 > PARAM_SIZE ? PARAM_SIZE : len - 3;
                memcpy(parameter, (char*)cmd_buffer + 3, paramlen);
                parameter[paramlen] = 0;
            }
            else
            {
                paramlen = 0;
            }
        }
        cmdCode = cmd_buffer[1];
        cmdCode <<= 8;
        cmdCode |= cmd_buffer[2];
        //Command rounting
        switch(cmdCode)
        {
            case CMD_NN://Loop back command
                printf("OK\r\n");
                break;
            case CMD_DT: //into DTM Mode (for RF teset)
                //com_port_function_set(COM_PORT_DTM);
                set_key_state_dtm_mode();
                printf("OK\r\n");
                break;
            case CMD_RR: //Launch a software reset
                sd_nvic_SystemReset();
                break;
            case CMD_HI: //Get MCU informations
                outputDeviceInfo();
                printf("UID:%08lX%08lX\r\n",NRF_FICR->DEVICEID[1],NRF_FICR->DEVICEID[0]);
                break;
            case CMD_SI: //Get firmware informations
                printf("Firmware Ver:%d.%d,Re:%s \r\n", VERSION, REVISON, __DATE__);
                printf("SoftDevice FWID:%d,ID:%ld,Ver:%ld\r\n",SD_FWID_GET(MBR_SIZE),SD_ID_GET(MBR_SIZE),
                        SD_VERSION_GET(MBR_SIZE));
                break;
            case CMD_RV: //Read VCC
                //adc_user_get_vcc(&vcc_voltage);
                //printf("VCC:%dmV\r\n", vcc_voltage);
                break;
            case CMD_SN: //Write and Read DEVICE ID
                if (paramlen == 12)
                {
                    parameter[12] = 0;//give string end
                    i = strtoull(parameter, &p, 16);
                    if (i == 0)
                    {
                        //Parameter error (not a nummber or zero).
                        printf("ERROR-3\r\n");
                    }
                    else
                    {
                        config->device_id[0] = i>>40;
                        config->device_id[1] = i>>32;
                        config->device_id[2] = i>>24;
                        config->device_id[3] = i>>16;
                        config->device_id[4] = i>>8;
                        config->device_id[5] = i>>0;

                        if (NRF_SUCCESS == store_config(config))
                        {
                            printf("OK\r\n");
                    //        flag_mac_base_update_request = true;
                        }
                        else
                        {
                            printf("ERROR-5\r\n");
                        }
                    }
                }
                else if (paramlen != 0)
                {
                    //Parameter length error.
                    printf("ERROR-1\r\n");
                }
                else
                {
                    printf("PCB_ID: %02X%02X%02X%02X%02X%02X\r\n", config->device_id[0],\
                            config->device_id[1],\
                            config->device_id[2],\
                            config->device_id[3],\
                            config->device_id[4],\
                            config->device_id[5]);
                }
                break;
            case CMD_DA: //Write and Read Date
                if (paramlen == 14)
                {
                    parameter[14] = 0;
                    tmp_64 = strtoull((char*)parameter, &p, 10);

                    current.year    = tmp_64/10000000000;
                    current.month   = tmp_64%10000000000/100000000;
                    current.day     = tmp_64%100000000/1000000;
                    current.hour    = tmp_64%1000000/10000;
                    current.min     = tmp_64%10000/100;
                    current.sec     = tmp_64%100;

                    uint32_t sum_sec = date_to_sec(current);
                    if (sync_time_by_sec(sum_sec))
                    {
                        printf("OK\r\n");
                    }
                    else
                    {
                        printf("ERROR -2\r\n");
                    }
                }
                else if (paramlen == 0)
                {
                    printf("%s\r\n", get_date_time());
                }
                else
                {
                    printf("ERROR -1\r\n");
                }
                break;
            case CMD_ED://Write and Read Beacon Seed
                if (paramlen == SEED_STR_LEN)
                {
                    char seed_str[SEED_STR_LEN+1];
                    memcpy(seed_str, parameter, SEED_STR_LEN);
                    seed_str[SEED_STR_LEN] = 0;
                    if (parse_seed_data(seed_str, get_config()->seed_data))
                    {
                        if (NRF_SUCCESS == store_config(get_config()))
                        {
                            //flag_mac_base_update_request = true;
                            //flag_beacon_mm_update_request = true;
                            printf("OK\r\n");
                        }
                        else
                        {
                            printf("ERROR-5\r\n");
                        }
                    }
                    else
                    {
                        printf("ERROR-6\r\n");
                    }
                }
                else if (paramlen != 0)
                {
                    printf("ERROR-1\r\n");
                }
                else
                {
                    printf("SEED: ");
                    for (i = 0; i < SEED_LEN; i++)
                    {
                        printf("%02X", get_config()->seed_data[i]);
                    }
                    printf("\r\n");
                }
                break;
            case CMD_UD:
                if (paramlen == UUID_STR_LEN)
                {
                    static char uuid_str[UUID_STR_LEN+1];
                    memcpy(uuid_str, parameter, UUID_STR_LEN);
                    uuid_str[UUID_STR_LEN] = 0;
                    parse_uuid_data(uuid_str, get_config()->uuid_normal);
                    if (NRF_SUCCESS == store_config(get_config()))
                    {
                        //flag_beacon_mm_update_request = true;
                        printf("OK\r\n");
                    }
                    else
                    {
                        printf("ERROR-5\r\n");
                    }
                }
                else if (paramlen != 0)
                {
                    printf("ERROR-1\r\n");
                }
                else
                {
                    printf("UUID_NORMAL: ");
                    for (i = 0; i < UUID_LEN; i++)
                    {
                        printf("%02X", get_config()->uuid_normal[i]);
                    }
                    printf("\r\n");
                }
                break;
            case CMD_UL:
                if(paramlen == UUID_STR_LEN)
                {
                    static char uuid_str[UUID_STR_LEN+1];
                    memcpy(uuid_str, parameter, UUID_STR_LEN);
                    uuid_str[UUID_STR_LEN] = 0;
                    parse_uuid_data(uuid_str, get_config()->uuid_low_battery);
                    if (NRF_SUCCESS == store_config(get_config()))
                    {
                        //flag_beacon_mm_update_request = true;
                        printf("OK\r\n");
                    }
                    else
                    {
                        printf("ERROR-5\r\n");
                    }
                }
                else if(paramlen != 0)
                {
                    printf("ERROR-1\r\n");
                }
                else
                {
                    printf("UUID_LOW_BATTERY: ");
                    for (i = 0; i < UUID_LEN; i++)
                    {
                        printf("%02X", get_config()->uuid_low_battery[i]);
                    }
                    printf("\r\n");
                }
                break;
            default:
                printf("Unkown Command.\r\n");
                break;
        }
    }
    memset(cmd_buffer, 0x0, UART_RX_MAX_DATA_LEN);
}

void uart_event_handle(app_uart_evt_t * p_event)
{
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            if (get_key_state()->is_dtm_mode)
                return;

            UNUSED_VARIABLE(app_uart_get(&cmd_buffer[index]));
            index++;

            if ((cmd_buffer[index - 1] == 0x08) && (index > 0))
            {
                index -= 2;
            }

            if ((cmd_buffer[index - 1] == '\n') || (cmd_buffer[index - 1] == '\r') || (index >= UART_RX_MAX_DATA_LEN - 1))
            {
                if (cmd_buffer[index - 1] != '\0')
                {
                    cmd_buffer[index - 1] = '\0';
                }

                NRF_LOG_HEXDUMP_INFO(cmd_buffer, index - 1);

                console_process();
                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            NRF_LOG_INFO("APP_UART_COMMUNICATION_ERROR");
            //APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            NRF_LOG_INFO("APP_UART_FIFO_ERROR");
            //APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
