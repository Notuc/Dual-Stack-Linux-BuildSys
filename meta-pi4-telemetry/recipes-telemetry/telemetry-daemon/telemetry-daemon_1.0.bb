SUMMARY = "CAN bus and BME280 telemetry daemon"
DESCRIPTION = "Reads BME280 sensor over I2C and publishes data over CAN and Unix socket"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://telemetry-daemon \
           file://telemetry-daemon.service"

S = "${WORKDIR}/telemetry-daemon"

inherit systemd

SYSTEMD_SERVICE:${PN} = "telemetry-daemon.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_compile() {
    ${CXX} ${CXXFLAGS} \
        -std=c++20 \
        -IDrivers/BME280 \
        -IDrivers/CAN \
        ${S}/Src/main.cpp \
        ${S}/Drivers/BME280/bme280.cpp \
        ${S}/Drivers/CAN/can_interface.cpp \
        ${LDFLAGS} -lpthread \
        -o ${S}/telemetry-daemon
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/telemetry-daemon ${D}${bindir}/telemetry-daemon

    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/telemetry-daemon.service ${D}${systemd_unitdir}/system/
}

FILES:${PN} += "${systemd_unitdir}/system/telemetry-daemon.service"
