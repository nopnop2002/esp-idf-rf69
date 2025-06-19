/* The example of RF69
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"

#include "rf69.h"

static const char *TAG = "MAIN";

MessageBufferHandle_t xMessageBufferTx;
MessageBufferHandle_t xMessageBufferRx;

// The total number of bytes (not single messages) the message buffer will be able to hold at any one time.
size_t xBufferSizeBytes = 1024;
// The size, in bytes, required to hold each item in the message,
size_t xItemSize = RH_RF69_MAX_MESSAGE_LEN; // Maximum Payload size of RF69 is RH_RF69_MAX_MESSAGE_LEN

#if CONFIG_SENDER
void tx_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	uint8_t packetData[xItemSize];
	while(1) {
		size_t packetLength = xMessageBufferReceive(xMessageBufferRx, packetData, sizeof(packetData), portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(NULL), "packetLength=%d", packetLength);
		ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(NULL), packetData, packetLength, ESP_LOG_DEBUG);
		send(packetData, (uint8_t)packetLength);
		waitPacketSent();
	} // end while

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_SENDER

#if CONFIG_RECEIVER
void rx_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	uint8_t packetData[xItemSize];
	while(1) {
		if (available()) {
			// Should be a message for us now	
			uint8_t packetLength = sizeof(packetData);
			if (recv(packetData, &packetLength)) {
				if (!packetLength) continue;
				ESP_LOGI(pcTaskGetName(NULL), "Received: [%.*s]", packetLength, (char*)packetData);
				ESP_LOGI(pcTaskGetName(NULL), "RSSI: %d", lastRssi());
				size_t spacesAvailable = xMessageBufferSpacesAvailable( xMessageBufferTx );
				ESP_LOGI(pcTaskGetName(NULL), "spacesAvailable=%d", spacesAvailable);
				size_t sended = xMessageBufferSend(xMessageBufferTx, packetData, packetLength, 100);
				if (sended != packetLength) {
					ESP_LOGE(pcTaskGetName(NULL), "xMessageBufferSend fail packetLength=%d sended=%d", packetLength, sended);
					break;
				}
			}
		} // end available
		vTaskDelay(1);
	} // end while

	vTaskDelete( NULL );
}
#endif // CONFIG_RECEIVER

void cdc_acm_vcp_task(void *pvParameters);

void app_main()
{
	// Create MessageBuffer
	xMessageBufferTx = xMessageBufferCreate(xBufferSizeBytes);
	configASSERT( xMessageBufferTx );
	xMessageBufferRx = xMessageBufferCreate(xBufferSizeBytes);
	configASSERT( xMessageBufferRx );

	// Initialize Radio
	if (!init()) {
		ESP_LOGE(TAG, "RFM69 radio init failed");
		while (1) { vTaskDelay(1); }
	}
	ESP_LOGI(TAG, "RFM69 radio init OK!");
  
	float freq;
#if CONFIG_RF69_FREQ_315
	freq = 315.0;
#elif CONFIG_RF69_FREQ_433
	freq = 433.0;
#elif CONFIG_RF69_FREQ_868
	freq = 868.0;
#elif CONFIG_RF69_FREQ_915
	freq = 915.0;
#endif
	ESP_LOGW(TAG, "Set frequency to %.1fMHz", freq);

	// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
	// No encryption
	if (!setFrequency(freq)) {
		ESP_LOGE(TAG, "setFrequency failed");
		while (1) { vTaskDelay(1); }
	}
	ESP_LOGI(TAG, "RFM69 radio setFrequency OK!");

#if CONFIG_RF69_POWER_HIGH
	// If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
	// ishighpowermodule flag set like this:
	setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW
	ESP_LOGW(TAG, "Set TX power high");
#endif

	// The encryption key has to be the same as the one in the server
	uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	setEncryptionKey(key);

#if CONFIG_SENDER
	xTaskCreate(&tx_task, "TX", 1024*3, NULL, 5, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&rx_task, "RX", 1024*3, NULL, 5, NULL);
#endif
	// Start CDC_ACM_VCP
	xTaskCreate(&cdc_acm_vcp_task, "CDC_ACM_VCP", 1024*4, NULL, 5, NULL);
}
