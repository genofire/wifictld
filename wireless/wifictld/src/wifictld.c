#include <libubox/uloop.h>
#include "log.h"
#include "wifi_clients.h"
#include "ubus_events.h"

int main(void)
{
	verbose = 1;
	client_probe_learning = 1;

	int ret = 0;

	#ifdef MINI
	log_info("start wifictld (mini)\n");
	#else
	log_info("start wifictld (full)\n");
	#endif

	log_debug("log: show debug\n");
	log_verbose("log: show verbose\n");

	uloop_init();

	// define wifi clients memory
	ret = wifi_clients_init();
	if (ret)
	{
		return ret;
	}
	// bind to loop
	ret = wifictld_ubus_init();
	if (ret)
	{
		wifi_clients_close();
		return ret;
	}
	uloop_run();
	uloop_done();


	wifictld_ubus_close();
	wifi_clients_close();

	return 0;
}
