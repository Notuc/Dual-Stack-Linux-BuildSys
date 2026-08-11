#include "can_interface.hpp"
#include <cstring>
#include <iostream>

int main() {
  try {
    // Two sockets on the same virtual bus: sender transmits, receiver
    // listens with no filter installed, so it should see every frame on
    // the bus, including this one.
    CanInterface sender("vcan0");
    CanInterface receiver("vcan0");

    // Only the first 4 bytes are meaningful (dlc = 4 below); the trailing
    // zeros are just padding within the 8-byte local buffer and are not
    // sent as part of the frame's payload.
    uint8_t payload[8] = {0x11, 0x22, 0x33, 0x44, 0, 0, 0, 0};
    if (!sender.sendFrame(0x123, payload, 4)) {
      std::cerr << "Send failed\n";
      return 1;
    }
    std::cout << "Sent frame ID 0x123\n";

    // Wait up to 1 second for the frame to arrive on the receiver socket.
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
    // Catches constructor failures from CanInterface (bad interface name,
    // interface not up, socket/bind failure, etc.)
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
