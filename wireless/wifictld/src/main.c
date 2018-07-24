#include "include.h"
#include "config.h"
#include "log.h"
#include "wifi_clients.h"
#include "ubus.h"

struct option longopts[] = {
	{"verbose", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	// Bandsteering options
	{"try", required_argument, 0, 't'},
	{"signal", required_argument, 0, 's'},
	{"force", optional_argument, 0, 'f'},
	{"force-probe", optional_argument, 0, 1},
	{"probe", optional_argument, 0, 'p'},
	{"probe-learning", optional_argument, 0, 2},
	// memory options
	{"clean-every", required_argument, 0, 3},
	{"clean-older-then", required_argument, 0, 4},
	{"clean-authed", optional_argument, 0, 5},
};

void usage(int c) {
	printf("Usage: wifictld ...\n");
	#ifdef DEBUG
	printf(" -v|--verbose         log verbose and debug (current: %s)\n", BOOL2STR(config_verbose));
	#else
	printf(" -v|--verbose         log verbose (current: %s)\n", BOOL2STR(config_verbose));
	#endif
	printf(" -h|--help            show this help text\n");
	printf("\nbandsteering options:\n");
	printf(" -t|--try             threashold till allow (current: %d)\n", config_client_try_threashold);
	printf(" -s|--signal          threashold to allow (current: %d)\n", config_client_signal_threashold);
	printf(" -f|--force           force steering (current: %s)\n", BOOL2STR(config_client_force));
	printf(" --force-probe        force steering only for probes (current: %s)\n", BOOL2STR(config_client_force_probe));
	printf(" -p|--probe           probe-steering (current: %s)\n", BOOL2STR(config_client_probe_steering));
	printf(" --probe-learning     probe-learing (current: %s)\n", BOOL2STR(config_client_probe_learning));
	printf("                      (parameter ignored if probe-steering on)\n");
	printf("\nmemory cleanup options:\n");
	printf(" --clean-every        run cleaning every (current: %ds)\n", config_client_clean_every);
	printf(" --clean-older-then   clean unseen for secound (current: %ds)\n", config_client_clean_older_then);
	printf(" --clean-authed       clean already authed also (not only learned by probes) (current: %s)\n", BOOL2STR(config_client_clean_authed));
	printf("\nConfig could be changed by ubus, see `ubus -v list wifictld`\n");
	if(c)
		printf("Invalid parameter %c ignored.\n", c);
}

int main(int argc, char *argv[])
{
	int ret = 0;

	#ifdef MINI
	log_info("start wifictld (mini)\n");
	#else
	log_info("start wifictld (full)\n");
	#endif

	int c = 0, indexptr = 0;
	while ((c = getopt_long(argc, argv, "vht:s:fp",longopts, &indexptr)) != -1) {
		switch (c) {
			case 'v':
				config_verbose = !config_verbose;
				break;
			case 'h':
				usage(0);
				return 0;
			// Bandsteering options
			case 't':
				config_client_try_threashold = atoi(optarg);
				if (config_client_try_threashold < 0) {
					printf("negativ tries count not supported\n");
					return 1;
				}
				break;
			case 's':
				config_client_signal_threashold = atoi(optarg);
				if (config_client_signal_threashold > 0) {
					printf("prositiv signal not supported\n");
					return 1;
				}
				break;
			case 'f':
				if(optarg)
					config_client_force = atoi(optarg);
				else
					config_client_force = !config_client_force;
				break;
			case 1:
				if(optarg)
					config_client_force_probe = atoi(optarg);
				else
					config_client_force_probe = !config_client_force_probe;
				break;
			case 'p':
				if(optarg)
					config_client_probe_steering = atoi(optarg);
				else
					config_client_probe_steering = !config_client_probe_steering;
				break;
			case 2:
				if(optarg)
					config_client_probe_learning = atoi(optarg);
				else
					config_client_probe_learning = !config_client_probe_learning;
				break;
			// memory options
			case 3:
				config_client_clean_every = atoi(optarg);
				if (config_client_clean_every < 0) {
					printf("negativ time not supported\n");
					return 1;
				}
				break;
			case 4:
				config_client_clean_older_then = atoi(optarg);
				if (config_client_clean_older_then < 0) {
					printf("negativ time not supported\n");
					return 1;
				}
				break;
			case 5:
				if(optarg)
					config_client_clean_authed = atoi(optarg);
				else
					config_client_clean_authed = !config_client_clean_authed;
				break;
			default:
				usage(c);
				return 1;
		}
	}

	log_debug("log: show debug\n");
	log_verbose("log: show verbose\n");

	uloop_init();

	// define wifi clients memory
	ret = wifi_clients_init();
	if (ret)
	{
		log_error("exit with error on client init\n");
		return ret;
	}
	// bind to loop
	ret = wifictld_ubus_init();
	if (ret)
	{
		wifi_clients_close();
		log_error("exit with error on ubus init\n");
		return ret;
	}
	uloop_run();
	uloop_done();


	wifictld_ubus_close();
	wifi_clients_close();

	log_info("safe exit\n");

	return 0;
}
