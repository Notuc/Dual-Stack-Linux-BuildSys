SUMMARY = "Telemetry logging service"
DESCRIPTION = "Receives telemetry data over Unix socket and writes to disk"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://logging-service \
           file://logging-service.service"

S = "${WORKDIR}/logging-service"

inherit systemd

SYSTEMD_SERVICE:${PN} = "logging-service.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_compile() {
    ${CXX} ${CXXFLAGS} \
        -std=c++20 \
        ${S}/Src/main.cpp \
        ${LDFLAGS} \
        -o ${S}/logging-service
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/logging-service ${D}${bindir}/logging-service

    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/logging-service.service ${D}${systemd_unitdir}/system/
}

FILES:${PN} += "${systemd_unitdir}/system/logging-service.service"
