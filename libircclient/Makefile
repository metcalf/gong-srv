include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libircclient
PKG_RELEASE:=1
PKG_VERSION:=1.7

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS := \
	-I$(STAGING_DIR)/usr/include/ \
	-I$(LINUX_DIR)/include \
	-fPIC \
	$(TARGET_CFLAGS)


CONFIGURE_ARGS = \
		--host=$(REAL_GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--program-prefix="" \
		--program-suffix="" \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--enable-shared

define Package/libircclient
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=IRC client
endef

define Package/libircclient/description
	IRC client
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./libircclient-1.7/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		$(TARGET_CONFIGURE_OPTS) \
		LDSHARED="$(TARGET_CC) -shared -Wl" \
		CFLAGS="$(TARGET_CFLAGS) $(FPIC)" \
		./configure $(CONFIGURE_ARGS) \
	);
endef

define Build/Compile
        $(MAKE) -C $(PKG_BUILD_DIR)/src \
        $(TARGET_CONFIGURE_OPTS) LDFLAGS="$(TARGET_LDFLAGS)" CFLAGS="$(TARGET_CFLAGS)"
endef

define Build/InstallDev
	echo "install dev"
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/libirc* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libircclient.so $(1)/usr/lib/
	#$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	#$(CP) ./files/ircclient.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libircclient/install
	echo "install"
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libircclient.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libircclient))
