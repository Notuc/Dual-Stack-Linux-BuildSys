#include "can_interface.hpp"
#include <cstring>
#include <linux/can/raw.h>
#include <net/if.h>
#include <poll.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

CanInterface::CanInterface(const std::string &ifname) {
  socket_fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (socket_fd_ < 0) {
    throw std::runtime_error("Failed to create CAN socket");
  }

  ifreq ifr{};
  std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ - 1);
  if (ioctl(socket_fd_, SIOCGIFINDEX, &ifr) < 0) {
    close(socket_fd_);
    throw std::runtime_error("Failed to find interface: " + ifname);
  }

  sockaddr_can addr{};
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(socket_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    close(socket_fd_);
    throw std::runtime_error("Failed to bind CAN socket");
  }
}

CanInterface::~CanInterface() {
  if (socket_fd_ >= 0)
    close(socket_fd_);
}

bool CanInterface::sendFrame(canid_t id, const uint8_t *data, uint8_t len) {
  can_frame frame{};
  frame.can_id = id;
  frame.can_dlc = len;
  std::memcpy(frame.data, data, len);

  return write(socket_fd_, &frame, sizeof(frame)) == sizeof(frame);
}

bool CanInterface::receiveFrame(can_frame &frame, int timeout_ms) {
  pollfd pfd{socket_fd_, POLLIN, 0};
  int ret = poll(&pfd, 1, timeout_ms);
  if (ret <= 0)
    return false;

  return read(socket_fd_, &frame, sizeof(frame)) == sizeof(frame);
}

bool CanInterface::setFilters(const std::vector<can_filter> &filters) {
  if (filters.empty())
    return false;

  int ret = setsockopt(socket_fd_, SOL_CAN_RAW, CAN_RAW_FILTER, filters.data(),
                       filters.size() * sizeof(can_filter));
  return ret == 0;
}
