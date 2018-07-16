include $(TOPDIR)/rules.mk

PKG_NAME:=wifictld
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Martin/Geno <geno+dev@fireorbit.de>
PKG_LICENSE:=APGL

include $(INCLUDE_DIR)/package.mk

define Package/wifictld/Default
	SECTION:=net
	CATEGORY:=Network
	TITLE:=wireless control daemon
	URL:=https://dev.sum7.eu/wifictld/wifictld
	DEPENDS:=+libubus +libubox
endef

define Package/wifictld
	$(call Package/wifictld/Default)
	TITLE+= (full)
	VARIANT:=full
	TARGET_CFLAGS += -DDEBUG
endef

define Package/wifictld-mini
	$(call Package/wifictld/Default)
	TITLE+= (local)
	VARIANT:=mini
	TARGET_CFLAGS += -DMINI
endef

define Package/wifictld/description
	$(call Package/wifictld-mini/Default)
endef

define Package/wifictld-mini/description
	This package is a wifi control daemon
	which use ubus to communicate with hostapd
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp ./src/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/wifictld/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifictld $(1)/usr/sbin/
endef

Package/wifictld-mini/install = $(Package/hostapd/install)


$(eval $(call BuildPackage,wifictld))
$(eval $(call BuildPackage,wifictld-mini))
