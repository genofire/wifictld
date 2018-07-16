# Wifi control deamon packages feed

## Description

This is an OpenWrt package feed containing community 
maintained wifictld packages.

## Usage

To use these packages, add the following line to the 
feeds.conf
in the OpenWrt buildroot:

```
src-git wifictld git@dev.sum7.eu:wifictld/wifictld.git
```

This feed should be included and enabled by default in the 
OpenWrt buildroot. To install all its package definitions, 
run:

```
./scripts/feeds update wifictld
./scripts/feeds install -a -p wifictld
```

The wifictld packages should now appear in menuconfig.

