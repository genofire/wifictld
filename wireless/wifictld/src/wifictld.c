#include <stdio.h>
#include <libubox/ulog.h>
#include <libubus.h>
#include "ubus_events.h"

int main(void)
{
	int ret = 0;
	#ifdef MINI
	ULOG_NOTE("start wifictld (mini)\n");
	#else
	ULOG_NOTE("start wifictld (full)\n");
	#endif

	uloop_init();

	//bind to loop
	ret = wifictld_ubus_init();

	if (ret)
	{
		return ret;
	}
	uloop_run();
	uloop_done();


	wifictld_ubus_close();

	return 0;
}
