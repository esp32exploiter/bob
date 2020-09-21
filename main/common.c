#include "common.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "string.h"
#include "nvs_flash.h"
#include "esp_wifi.h"

void init_fucking_nvs_flash() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  }
}

void init_fucking_net_interface() {
  ESP_ERROR_CHECK(esp_netif_init());
}

void init_fucking_wifi() {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  // Init dummy AP to specify a channel and get WiFi hardware into
  // a mode where we can send the actual fake beacon frames.
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  wifi_config_t ap_config = {.ap = {.ssid = "esp32-beaconspam",
                                    .ssid_len = 0,
                                    .password = "dummypassword",
                                    .channel = 1,
                                    .authmode = WIFI_AUTH_WPA2_PSK,
                                    .ssid_hidden = 1,
                                    .max_connection = 4,
                                    .beacon_interval = 60000}};

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
}

void start_fucking_wifi() {
  ESP_ERROR_CHECK(esp_wifi_start());
}

#define BEACON_SSID_OFFSET 38
#define SRCADDR_OFFSET 10
#define BSSID_OFFSET 16

uint8_t beacon_raw[] = {
    0x80, 0x00, // 0-1: Frame Control
    0x00, 0x00, // 2-3: Duration
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 4-9: Destination address (broadcast)
    0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06, // 10-15: Source address
    0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06, // 16-21: BSSID
    0x00, 0x00, // 22-23: Sequence / fragment number
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 24-31: Timestamp (GETS OVERWRITTEN TO 0 BY HARDWARE)
    0x64, 0x00, // 32-33: Beacon interval
    0x31, 0x04, // 34-35: Capability info
    0x00, 0x00, /* FILL CONTENT HERE */ // 36-38: SSID parameter set, 0x00:length:content
    0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24, // 39-48: Supported rates
    0x03, 0x01, 0x01, // 49-51: DS Parameter set, current channel 1 (= 0x01),
    0x05, 0x04, 0x01, 0x02, 0x00, 0x00, // 52-57: Traffic Indication Map
};

uint64_t create_fucking_beacon_package(uint8_t beacon_rick[200], const char* ssid, uint8_t id) {
  memcpy(beacon_rick, beacon_raw, BEACON_SSID_OFFSET - 1);
  beacon_rick[BEACON_SSID_OFFSET - 1] = strlen(ssid);
  memcpy(&beacon_rick[BEACON_SSID_OFFSET], ssid,
         strlen(ssid));
  memcpy(&beacon_rick[BEACON_SSID_OFFSET + strlen(ssid)],
         &beacon_raw[BEACON_SSID_OFFSET],
         sizeof(beacon_raw) - BEACON_SSID_OFFSET);

  // Last byte of source address / BSSID will be line number - emulate multiple
  // APs broadcasting one song line each
  beacon_rick[SRCADDR_OFFSET + 5] = id;
  beacon_rick[BSSID_OFFSET + 5] = id;

  return sizeof(beacon_raw) + strlen(ssid);
}

uint64_t create_fucking_data_packet(uint8_t *data_rick, uint32_t id) {
  uint8_t data_frame[] = {
    0x08, 0x02,                         // Frame control 0 1
    0x00, 0x00,                         // Duration      2 3
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination address 4 5 6 7 8 9
    0x00, 0x12, 0x34, 0x56, 0x78, 0x9b, // Source address      10 11 12 13 14 15
    0x00, 0x12, 0x34, 0x56, 0x78, 0x9b, 0x00, 0x00,  // 16 17 18 19 20 21 22 23
    0x12, 0x34, 0x56, 0x78,  // Payload 24 25 26 27
    0x29, 0x62, 0xba, 0x76,
  };
  memcpy(data_rick, &data_frame, 32);
  data_rick[24] = id & 0xff;
  data_rick[25] = (id >> 8) & 0xff;
  data_rick[26] = (id >> 16) & 0xff;
  data_rick[27] = (id >> 24) & 0xff;
  return 32;
}
