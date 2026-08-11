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
  // PF_CAN / SOCK_RAW / CAN_RAW: a raw CAN socket, Each frame
  // written/read is a full struct can_frame (id + dlc + up to 8 data
  // bytes)
  socket_fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (socket_fd_ < 0) {
    throw std::runtime_error("Failed to create CAN socket");
  }

  // Resolve the human-readable interface name (e.g. "vcan0") to a kernel
  // interface index via the standard SIOCGIFINDEX ioctl — the same
  // mechanism used for any Linux network interface, not CAN-specific
  ifreq ifr{};
  std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ - 1);
  if (ioctl(socket_fd_, SIOCGIFINDEX, &ifr) < 0) {
    close(socket_fd_);
    throw std::runtime_error("Failed to find interface: " + ifname);
  }

  // Bind the socket to that specific CAN interface. sockaddr_can is the
  // CAN-specific sockaddr variant
  // can_ifindex is the only field we need to set for a raw, unfiltered
  // bind to one interface
  sockaddr_can addr{};
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(socket_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    close(socket_fd_);
    throw std::runtime_error("Failed to bind CAN socket");
  }
}

// Release the socket fd. Nothing CAN-specific here — same cleanup pattern
// as closing any other socket or file descriptor.
CanInterface::~CanInterface() {
  if (socket_fd_ >= 0)
    close(socket_fd_);
}

// Builds a struct can_frame from the given id/data/len and writes it to
// the socket in one syscall. So comparing the write() return value against
// sizeof(frame) is a reliable success check.
bool CanInterface::sendFrame(canid_t id, const uint8_t *data, uint8_t len) {
  can_frame frame{};
  frame.can_id = id;
  frame.can_dlc =
      len; // Data Length Code: number of valid bytes in frame.data (0-8)
  std::memcpy(frame.data, data, len);

  return write(socket_fd_, &frame, sizeof(frame)) == sizeof(frame);
}

// poll() to wait (with a timeout) for the socket to become readable,
// rather than blocking indefinitely on read()
bool CanInterface::receiveFrame(can_frame &frame, int timeout_ms) {
  // POLLIN: wake up as soon as there's data available to read on this fd.
  pollfd pfd{socket_fd_, POLLIN, 0};
  int ret = poll(&pfd, 1, timeout_ms);
  if (ret <= 0)
    // ret == 0: timed out with nothing available.
    // ret <  0: poll() itself errored (e.g. interrupted syscall).
    // Either way, no frame to return.
    return false;

  // Data is available — read exactly one frame's worth.
  // CAN reads are all-or-nothing for a given can_frame, so this equality
  // check is sufficient to detect a successful, complete read.
  return read(socket_fd_, &frame, sizeof(frame)) == sizeof(frame);
}

// Installs a kernel-level receive filter list on this socket via
// setsockopt(SOL_CAN_RAW, CAN_RAW_FILTER, ...). Each can_filter is an
// {can_id, can_mask} pair; a frame is delivered to this socket if
// (frame.can_id & mask) == (filter.can_id & mask) for at least one
// filter in the list (i.e. filters are OR'd together).
bool CanInterface::setFilters(const std::vector<can_filter> &filters) {
  if (filters.empty())
    return false;

  int ret = setsockopt(socket_fd_, SOL_CAN_RAW, CAN_RAW_FILTER, filters.data(),
                       filters.size() * sizeof(can_filter));
  return ret == 0;
}
