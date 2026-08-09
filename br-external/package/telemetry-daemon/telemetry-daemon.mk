TELEMETRY_DAEMON_VERSION = 1.0
TELEMETRY_DAEMON_SITE = $(BR2_EXTERNAL_TELEMETRY_PATH)/../telemetry-daemon
TELEMETRY_DAEMON_SITE_METHOD = local


define TELEMETRY_DAEMON_BUILD_CMDS
	$(MAKE) CXX="$(TARGET_CXX)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		-C $(@D)
endef

define TELEMETRY_DAEMON_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/daemon \
		$(TARGET_DIR)/usr/bin/telemetry-daemon
	$(INSTALL) -D -m 0644 \
		$(BR2_EXTERNAL_TELEMETRY_PATH)/package/telemetry-daemon/telemetry-daemon.service \
		$(TARGET_DIR)/etc/systemd/system/telemetry-daemon.service
endef

define TELEMETRY_DAEMON_INSTALL_INIT_SYSTEMD
        mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -sf /etc/systemd/system/telemetry-daemon.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/telemetry-daemon.service
endef

$(eval $(generic-package))
