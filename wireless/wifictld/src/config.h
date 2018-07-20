#ifndef __WIFICTLD_CONFIG_H
#define __WIFICTLD_CONFIG_H

#include "include.h"

#define WIFI_CLIENT_FREQ_THREASHOLD 5000

extern int verbose;

extern int client_try_threashold;
extern int client_signal_threashold;

extern int clean_every;
extern int clean_older_then;

// force (disable 2.4 Ghz)
extern bool client_force;
// force (disable 2.4 Ghz) for wifi probes
extern bool client_force_probe;
// use client_try_threashold for probes, too
extern bool client_probe_steering;
/*
	steering contains learning already:
	when client_probe_steering is set,
	client_probe_learning is ignored
*/
extern bool client_probe_learning;

#endif
