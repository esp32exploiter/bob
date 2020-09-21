#ifndef _COMMON_H_
#define _COMMON_H_

#include "esp_err.h"

void init_fucking_nvs_flash();
void init_fucking_net_interface();
void init_fucking_wifi();
void start_fucking_wifi();

uint64_t create_fucking_beacon_package(uint8_t beacon_rick[200],
                                       const char *ssid, uint8_t id);
uint64_t create_fucking_data_packet(uint8_t data_rick[200], uint32_t id);

struct wifi_80211_packet_t {
  uint8_t header[24];
  uint8_t payload[4];
  uint8_t shit[4];
};

#endif
