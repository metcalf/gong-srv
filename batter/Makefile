include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=batter
PKG_RELEASE:=1
PKG_VERSION:=0.1

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS := \
	-I$(STAGING_DIR)/usr/include/ \
	-I$(LINUX_DIR)/include \
	$(TARGET_CFLAGS)

define Package/batter
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:= Utility for swinging a servo over GPIO
  DEPENDS:= +kmod-pwm-gpio-custom +libuci
endef

define Package/batter/description
	Utility for swinging a servo over GPIO
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
        $(MAKE) -C $(PKG_BUILD_DIR) \
        $(TARGET_CONFIGURE_OPTS) LDFLAGS="$(TARGET_LDFLAGS) -lm -luci" CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/batter/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/batter.config $(1)/etc/config/batter
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/batter $(1)/usr/sbin/batter
endef

$(eval $(call BuildPackage,batter))
