#include "common.h"
#include "esp_interface.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "string.h"

void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_DATA)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  // Our packet is greater than 32.
  if (ppkt->rx_ctrl.sig_len < 32)
    return;
  const uint8_t *payload = ppkt->payload;
  // 08 02 00 00 ff ff ff ff ff ff 00 12 34 56 78 9b 00 12 34 56 78 9b 40 3c d2
  // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
  // 04 00 00 29 62 ba 76
  // 25 26 27 28 29 30 31
  if (!(payload[10] == 0x00 &&
        payload[11] == 0x12 &&
        payload[12] == 0x34 &&
        payload[13] == 0x56 &&
        payload[14] == 0x78 &&
        payload[15] == 0x9b))
    return;
  uint32_t index = 0;
  index += (payload[24]);
  index += (payload[25] << 8);
  index += (payload[26] << 16);
  index += (payload[27] << 24);
  esp_wifi_80211_tx(ESP_IF_WIFI_AP, payload, 32, true);
  printf("%u,%d\n", index, ppkt->rx_ctrl.rssi);
}

void app_main() {
  init_fucking_nvs_flash();
  init_fucking_net_interface();
  init_fucking_wifi();
  start_fucking_wifi();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
}
