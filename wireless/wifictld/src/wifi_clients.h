#ifndef __WIFICTLD_WIFI_CLIENTS_H
#define __WIFICTLD_WIFI_CLIENTS_H

#include <stdint.h>

extern int client_freq_try_threashold;

#define WIFI_CLIENT_FREQ_THREASHOLD 5000

int wifi_clients_init();
void wifi_clients_close();

void wifi_clients_learn(const uint8_t *address, uint32_t freq);
int wifi_clients_try(const uint8_t *address, uint32_t freq);

#endif
