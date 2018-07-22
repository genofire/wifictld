#ifndef __WIFICTLD_DATA_H
#define __WIFICTLD_DATA_H

#include "include.h"

extern struct avl_tree clients_by_addr;


struct wifi_client {
	struct avl_node avl;
	u8 addr[ETH_ALEN];
	time_t time;
	int try_probe;
	int try_auth;
	bool connected;
	bool authed;
	uint32_t freq_highest;
	uint32_t signal_lowfreq;
	uint32_t signal_highfreq;
};

#endif
