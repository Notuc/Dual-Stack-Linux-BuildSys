LOGGING_SERVICE_VERSION = 1.0
LOGGING_SERVICE_SITE = $(BR2_EXTERNAL_TELEMETRY_PATH)/../logging-service
LOGGING_SERVICE_SITE_METHOD = local

define LOGGING_SERVICE_BUILD_CMDS
	$(MAKE) CXX="$(TARGET_CXX)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		-C $(@D)
endef

define LOGGING_SERVICE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/logging-service \
		$(TARGET_DIR)/usr/bin/logging-service
	$(INSTALL) -D -m 0644 \
		$(BR2_EXTERNAL_TELEMETRY_PATH)/package/logging-service/logging-service.service \
		$(TARGET_DIR)/etc/systemd/system/logging-service.service
endef

define LOGGING_SERVICE_INSTALL_INIT_SYSTEMD
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -sf /etc/systemd/system/logging-service.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/logging-service.service
endef

$(eval $(generic-package))
