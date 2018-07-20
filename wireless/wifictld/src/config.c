#include "config.h"

bool config_verbose = false;

int config_client_try_threashold = 3;
int config_client_signal_threashold = -75;

int config_client_clean_every = 600; //in ms  = 10min
int config_client_clean_older_then = 3600;  //in sec = 1h
bool config_client_clean_authed = false;

bool config_client_force = false;
bool config_client_force_probe = false;
bool config_client_probe_steering = true;
// steering contains learning already
bool config_client_probe_learning = false;
