#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char* DEMO_TAG = "Akash_v1";
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
static const char remote_device_name[] = "Akash";

static bool connect    = false;
static bool get_server = false;


static esp_ble_scan_params_t ble_scan_params = {
	.scan_type              = BLE_SCAN_TYPE_ACTIVE,
	.own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
	.scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
	.scan_interval          = 0x50,
	.scan_window            = 0x30,
	.scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param)
{
	esp_err_t err;
	uint8_t *adv_name = NULL;
	uint8_t adv_name_len = 0;

	switch(event)
	{
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
			{
				uint32_t duration = 0;
				esp_ble_gap_start_scanning(duration);
				break;
			}
		case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
			{
				if((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
					ESP_LOGE(DEMO_TAG,"Scan start failed: %s", esp_err_to_name(err));
				}
				else {
					ESP_LOGI(DEMO_TAG,"Start scanning...");
				}
				break;
			}

		case ESP_GAP_BLE_SCAN_RESULT_EVT:
			{
				esp_ble_gap_cb_param_t* scan_result = (esp_ble_gap_cb_param_t*)param;
				switch(scan_result->scan_rst.search_evt)
				{
					case ESP_GAP_SEARCH_INQ_RES_EVT:
						{
							esp_log_buffer_hex(DEMO_TAG, scan_result->scan_rst.bda, 6);
							ESP_LOGI(DEMO_TAG, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
							adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
									ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
							ESP_LOGI(DEMO_TAG, "searched Device Name Len %d", adv_name_len);
							esp_log_buffer_char(DEMO_TAG, adv_name, adv_name_len);
							ESP_LOGI(DEMO_TAG, "\n");
							if (adv_name != NULL) {
								if (strlen(remote_device_name) == adv_name_len && strncmp((char *)adv_name, remote_device_name, adv_name_len) == 0) {
									ESP_LOGI(DEMO_TAG, "searched device %s\n", remote_device_name);
									if (connect == false) {
										connect = true;
										ESP_LOGI(DEMO_TAG, "connect to the remote device.");
										esp_ble_gap_stop_scanning();
										//esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);
									}
								}
							}
							break;
							case ESP_GAP_SEARCH_INQ_CMPL_EVT:
							break;
							default:
							break;
						}
						break;
				}
				case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
				{
					if((err = param->scan_stop_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
						ESP_LOGE(DEMO_TAG,"Scan stop failed: %s", esp_err_to_name(err));
					}
					else {
						ESP_LOGI(DEMO_TAG,"Stop scan successfully");
					}
					break;
				}
				default:
				break;
			}
	}
}

	void app_main()
	{
		ESP_ERROR_CHECK(nvs_flash_init());
		ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
		esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
		esp_bt_controller_init(&bt_cfg);
		esp_bt_controller_enable(ESP_BT_MODE_BLE);

		esp_bluedroid_init();
		esp_bluedroid_enable();

		esp_err_t status;

	ESP_LOGI(DEMO_TAG,"Register callback");

	/*<! register the scan callback function to the gap module */
	if((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK) {
		ESP_LOGE(DEMO_TAG,"gap register error: %s", esp_err_to_name(status));
		return;
	}

}
