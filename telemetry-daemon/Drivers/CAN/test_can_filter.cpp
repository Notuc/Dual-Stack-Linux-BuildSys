#include "can_interface.hpp"
#include <iostream>

int main() {
  CanInterface sender("vcan0");
  CanInterface receiver("vcan0");

  can_filter filter{};
  filter.can_id = 0x123;
  filter.can_mask = CAN_SFF_MASK;
  receiver.setFilters({filter});

  uint8_t payload[4] = {0xAA, 0xBB, 0xCC, 0xDD};

  // This should be filtered out
  sender.sendFrame(0x456, payload, 4);
  can_frame frame{};
  if (receiver.receiveFrame(frame, 500)) {
    std::cout << "UNEXPECTED: received filtered-out ID 0x" << std::hex
              << frame.can_id << "\n";
  } else {
    std::cout << "Correctly filtered out 0x456\n";
  }

  // This should pass through
  sender.sendFrame(0x123, payload, 4);
  if (receiver.receiveFrame(frame, 500)) {
    std::cout << "Correctly received 0x" << std::hex << frame.can_id << "\n";
  } else {
    std::cout << "UNEXPECTED: 0x123 not received\n";
  }

  return 0;
}
