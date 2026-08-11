# Buildroot vs Yocto Comparison

## Build System
| Metric | Buildroot | Yocto |
|---|---|---|
| Version | 2026.05 | Scarthgap 5.0.19 |
| Kernel | linux-rpi custom | linux-raspberrypi 6.6.63 |
| Init system | systemd | systemd |
| Build time (cold) | ~1.5 hours | ~4 hours |

## Image Metrics
| Metric | Buildroot | Yocto |
|---|---|---|
| Image size | 153MB | 58MB compressed|
| Rootfs used | 77.7MB | 119MB |
| Boot to targets | ~4.5 seconds | ~2.4 seconds |
| RAM at idle | 54.1MB | 85MB |

## Application Stack
| Component | Buildroot | Yocto |
|---|---|---|
| telemetry-daemon | systemd service | systemd service |
| logging-service | systemd service | systemd service |
| CAN interface | MCP2515 HAT | MCP2515 HAT |
| I2C sensor | BME280 | BME280 |

## Developer Experience
| Aspect | Buildroot | Yocto |
|---|---|---|
| Learning curve | Moderate | Steep |
| Build speed | Fast | Slow |
| Reproducibility | Good | Excellent |
| Extensibility | Limited | Very high |
| Industry usage | Industrial/embedded | Automotive/consumer |
| Package management | None | opkg/rpm |

## Key Differences
- Buildroot produces a smaller, faster image with less configuration
- Yocto takes longer to build but is more reproducible and extensible
- Both use the same application code — only the build system differs
