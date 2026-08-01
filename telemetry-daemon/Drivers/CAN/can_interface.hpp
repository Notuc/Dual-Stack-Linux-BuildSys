#pragma once
#include <cstdint>
#include <linux/can.h>
#include <string>
#include <vector>

class CanInterface {
public:
  explicit CanInterface(const std::string &ifname);
  ~CanInterface();

  bool sendFrame(canid_t id, const uint8_t *data, uint8_t len);
  bool receiveFrame(can_frame &frame, int timeout_ms = 100);

  // Restrict this socket to only receive frames matching these ID/mask pairs.
  // Call before receiveFrame(); if never called, all frames are received.
  bool setFilters(const std::vector<can_filter> &filters);

private:
  int socket_fd_ = -1;
};
