/* The example of ESP-IDF
 *
 * This sample code is in the public domain.
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "rf69.h"

#define TAG "MAIN"

#if CONFIG_TRANSMITTER
void tx_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	int packetnum = 0;	// packet counter, we increment per xmission
	while(1) {

		char radiopacket[64] = "Hello World #";
		sprintf(radiopacket, "Hello World #%d", packetnum++);
		ESP_LOGI(pcTaskGetName(0), "Sending %s", radiopacket);
  
		// Send a message!
		send((uint8_t *)radiopacket, strlen(radiopacket));
		waitPacketSent();

		// Now wait for a reply
		uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
		uint8_t len = sizeof(buf);

		if (waitAvailableTimeout(500))	{
			// Should be a reply message for us now   
			if (recv(buf, &len)) {
				ESP_LOGI(pcTaskGetName(0), "Got a reply: %s", (char*)buf);
			} else {
				ESP_LOGE(pcTaskGetName(0), "Receive failed");
			}
		} else {
			ESP_LOGE(pcTaskGetName(0), "No reply, is another RFM69 listening?");
		}
		vTaskDelay(1000/portTICK_PERIOD_MS);
	} // end while

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_TRANSMITTER

#if CONFIG_RECEIVER
void rx_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetName(0), "Start");

	while(1) {

		if (available()) {
			// Should be a message for us now	
			uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
			uint8_t len = sizeof(buf);
			if (recv(buf, &len)) {
				if (!len) continue;
				buf[len] = 0;
				ESP_LOGI(pcTaskGetName(0), "Received [%d]:%s", len, (char*)buf);
				ESP_LOGI(pcTaskGetName(0), "RSSI: %d", lastRssi());

				if (strstr((char *)buf, "Hello World")) {
					// Send a reply!
					uint8_t data[] = "And hello back to you";
					send(data, sizeof(data));
					waitPacketSent();
					ESP_LOGI(pcTaskGetName(0), "Sent a reply");
				}
			} else {
				ESP_LOGE(pcTaskGetName(0), "Receive failed");
			} // end recv
		} // end available
		vTaskDelay(1);
	} // end while

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_RECEIVER

void app_main()
{
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

#if CONFIG_TRANSMITTER
	xTaskCreate(&tx_task, "tx_task", 1024*3, NULL, 1, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&rx_task, "rx_task", 1024*3, NULL, 1, NULL);
#endif
}
