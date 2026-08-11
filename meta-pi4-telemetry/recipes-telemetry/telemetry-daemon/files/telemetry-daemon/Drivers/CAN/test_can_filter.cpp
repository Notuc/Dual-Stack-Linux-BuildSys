#include "can_interface.hpp"
#include <iostream>

int main() {
  // Two independent sockets bound to the same virtual bus: one acts as
  // the transmitter, one as the receiver under test. Because both are
  // bound to "vcan0", the kernel delivers sender's frames to receiver
  // just as it would with two separate processes/nodes on a real bus.
  CanInterface sender("vcan0");
  CanInterface receiver("vcan0");

  // Install a filter on the receiver that only accepts frames with
  // can_id == 0x123 exactly (CAN_SFF_MASK = full 11-bit standard-frame
  // ID mask
  can_filter filter{};
  filter.can_id = 0x123;
  filter.can_mask = CAN_SFF_MASK;
  receiver.setFilters({filter});

  uint8_t payload[4] = {0xAA, 0xBB, 0xCC, 0xDD};

  // Negative case: a frame with a non-matching ID should be dropped
  // by the kernel filter before it ever reaches receiver's read queue
  // This should be filtered out
  sender.sendFrame(0x456, payload, 4);
  can_frame frame{};
  if (receiver.receiveFrame(frame, 500)) {
    std::cout << "UNEXPECTED: received filtered-out ID 0x" << std::hex
              << frame.can_id << "\n";
  } else {
    std::cout << "Correctly filtered out 0x456\n";
  }

  //  Positive case: a frame with a matching ID should pass straight
  // through the filter and be received normally
  // This should pass through
  sender.sendFrame(0x123, payload, 4);
  if (receiver.receiveFrame(frame, 500)) {
    std::cout << "Correctly received 0x" << std::hex << frame.can_id << "\n";
  } else {
    std::cout << "UNEXPECTED: 0x123 not received\n";
  }

  return 0;
}
