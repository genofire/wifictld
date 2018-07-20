#ifndef __WIFICTLD_CONFIG_H
#define __WIFICTLD_CONFIG_H

#include "include.h"

#define WIFI_CLIENT_FREQ_THREASHOLD 5000

extern bool config_verbose;

extern int config_client_try_threashold;
extern int config_client_signal_threashold;

extern int config_client_clean_every;
extern int config_client_clean_older_then;
extern bool config_client_clean_authed;

// force (disable 2.4 Ghz)
extern bool config_client_force;
// force (disable 2.4 Ghz) for wifi probes
extern bool config_client_force_probe;
// use client_try_threashold for probes, too
extern bool config_client_probe_steering;
/*
	steering contains learning already:
	when client_probe_steering is set,
	client_probe_learning is ignored
*/
extern bool config_client_probe_learning;

#endif
