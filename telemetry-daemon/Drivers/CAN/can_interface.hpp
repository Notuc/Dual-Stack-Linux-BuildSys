#pragma once
#include <cstdint>
#include <linux/can.h>
#include <string>
#include <vector>

class CanInterface {
public:
  // Opens a CAN_RAW socket and binds it to the named SocketCAN interface
  // (e.g. "can0" for a physical bus/transceiver, "vcan0" for a virtual bus)
  explicit CanInterface(const std::string &ifname);

  // Closes the underlying socket file descriptor, if open.
  ~CanInterface();

  // Transmits a single classic CAN frame.
  //   id   - CAN identifier. Use an 11-bit value for a standard frame, or
  //          OR in CAN_EFF_FLAG with a 29-bit value for an extended frame
  //   data - pointer to the payload bytes to send.
  //   len  - payload length in bytes; classic CAN allows 0-8.
  //
  // Returns true if the frame was written to the socket successfully
  bool sendFrame(canid_t id, const uint8_t *data, uint8_t len);

  // Blocks (via poll()) for up to timeout_ms waiting for a frame to
  // become available on this socket, then reads one frame into `frame`
  // if one arrived in time.
  //
  //   frame      - output parameter; populated only if this function returns
  //   true. timeout_ms - how long to wait for a frame before giving. Defaults
  //   to 100ms.
  //
  // Returns true if a full can_frame was successfully read before the timeout
  // elapsed
  bool receiveFrame(can_frame &frame, int timeout_ms = 100);

  // Restrict this socket to only receive frames matching these ID/mask pairs.
  // Call before receiveFrame(); if never called, all frames are received.
  bool setFilters(const std::vector<can_filter> &filters);

private:
  // File descriptor for the underlying CAN_RAW socket. -1 would indicate "not
  // open,"
  int socket_fd_ = -1;
};
