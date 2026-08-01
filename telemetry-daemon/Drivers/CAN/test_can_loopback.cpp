// test_can_loopback.cpp
#include "can_interface.hpp"
#include <cstring>
#include <iostream>

int main() {
  try {
    CanInterface sender("vcan0");
    CanInterface receiver("vcan0");

    uint8_t payload[8] = {0x11, 0x22, 0x33, 0x44, 0, 0, 0, 0};
    if (!sender.sendFrame(0x123, payload, 4)) {
      std::cerr << "Send failed\n";
      return 1;
    }
    std::cout << "Sent frame ID 0x123\n";

    can_frame received{};
    if (receiver.receiveFrame(received, 1000)) {
      std::cout << "Received ID 0x" << std::hex << received.can_id
                << " len=" << std::dec << (int)received.can_dlc << " data=";
      for (int i = 0; i < received.can_dlc; i++)
        std::cout << std::hex << (int)received.data[i] << " ";
      std::cout << "\n";
    } else {
      std::cerr << "No frame received (timeout)\n";
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
