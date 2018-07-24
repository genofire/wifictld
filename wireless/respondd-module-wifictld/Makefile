include $(TOPDIR)/rules.mk

PKG_NAME:=respondd-module-wifictld
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_LICENSE:=BSD-2-Clause

PKG_BUILD_DEPENDS := respondd

include $(INCLUDE_DIR)/package.mk

define Package/respondd-module-wifictld
  SECTION:=wireless
  CATEGORY:=Network
  TITLE:=Add wifictld information to respondd
  DEPENDS:=+respondd +libubus +libubox
endef

define Package/respondd-module-wifictld/install
	$(INSTALL_DIR) $(1)/usr/lib/respondd
	$(CP) $(PKG_BUILD_DIR)/respondd.so $(1)/usr/lib/respondd/wifictld.so
endef

$(eval $(call BuildPackage,respondd-module-wifictld))
