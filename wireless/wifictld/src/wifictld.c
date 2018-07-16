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
	log_error("start wifictld (mini)\n");
	#else
	log_error("start wifictld (full)\n");
	#endif

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
