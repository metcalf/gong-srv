include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pitcher
PKG_RELEASE:=1
PKG_VERSION:=0.1

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS := \
	-I$(STAGING_DIR)/usr/include/ \
	-I$(LINUX_DIR)/include \
	$(TARGET_CFLAGS)

define Package/pitcher
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:= Utility for running commands on GPIO events
  DEPENDS:= +libuci +libircclient
endef

define Package/pitcher/description
	Utility for running commands on GPIO events
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
        $(MAKE) -C $(PKG_BUILD_DIR) \
        $(TARGET_CONFIGURE_OPTS) LDFLAGS="$(TARGET_LDFLAGS) -lm -luci" CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/pitcher/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/hotplug.d/button
	$(INSTALL_CONF) ./files/pitcher.config $(1)/etc/config/pitcher
	$(INSTALL_CONF) ./files/01-pitch $(1)/etc/hotplug.d/button/01-pitch
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pitcher $(1)/usr/sbin/pitcher
endef

$(eval $(call BuildPackage,pitcher))
