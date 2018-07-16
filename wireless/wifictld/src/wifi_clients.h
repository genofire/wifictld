#ifndef __WIFICTLD_WIFI_CLIENTS_H
#define __WIFICTLD_WIFI_CLIENTS_H

#include <stdint.h>

static int client_freq_try_threashold = 3;

int wifi_clients_init();
void wifi_clients_close();

void wifi_clients_learn(const uint8_t *address, uint32_t freq);
int wifi_clients_try(const uint8_t *address, uint32_t freq);

#endif
