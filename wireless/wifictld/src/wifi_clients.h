#ifndef __WIFICTLD_WIFI_CLIENTS_H
#define __WIFICTLD_WIFI_CLIENTS_H

#include "include.h"

int wifi_clients_init();
void wifi_clients_close();

void wifi_clients_learn(const uint8_t *address, uint32_t freq, uint32_t ssi_signal);
int wifi_clients_try(bool auth, const uint8_t *address, uint32_t freq, uint32_t ssi_signal);
void wifi_clients_disconnect(const uint8_t *address, uint32_t freq, uint32_t ssi_signal);

void wifi_clients_del(const u8 *addr);

#endif
