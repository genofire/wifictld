#include "config.h"

int verbose = 0;

int client_try_threashold = 3;
int client_signal_threashold = -75;

int clean_every = 600; //in ms  = 10min
int clean_older_then = 3600;  //in sec = 1h

bool client_force = false;
bool client_force_probe = false;
bool client_probe_steering = true;
// steering contains learning already
bool client_probe_learning = false;
