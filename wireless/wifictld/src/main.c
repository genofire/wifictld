#include "include.h"
#include "config.h"
#include "log.h"
#include "wifi_clients.h"
#include "ubus.h"

int main(int argc, char *argv[])
{
	int ret = 0;

	#ifdef MINI
	log_info("start wifictld (mini)\n");
	#else
	log_info("start wifictld (full)\n");
	#endif

	int c = 0;
	while ((c = getopt(argc, argv, "v")) != -1) {
		switch (c) {
			case 'v':
				verbose = 1;
				break;
			default:
				log_error("Invalid parameter %c ignored.\n", c);
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
