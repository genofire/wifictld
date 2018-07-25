#ifndef __WIFICTLD_INCLUDE_H
#define __WIFICTLD_INCLUDE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <libubus.h>
#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <libubox/uloop.h>
#include <libubox/blobmsg.h>
#include <getopt.h>
#include "hostapd/ieee802_11_defs.h" // ETH_ALEN + hwaddr_aton
#include "hostapd/common.h"

#define blobmsg_add_bool blobmsg_add_u8
#endif
