#pragma once

#include <cstddef>
#include <cstdint>

#include "Communication.h"

class SevenBitEncodedCommunication {
  public:
    SevenBitEncodedCommunication(Communication& inner, size_t txSize, size_t rxSize);
    ~SevenBitEncodedCommunication();

    SevenBitEncodedCommunication(const SevenBitEncodedCommunication&) = delete;
    SevenBitEncodedCommunication& operator=(const SevenBitEncodedCommunication&) = delete;

    SevenBitEncodedCommunication(SevenBitEncodedCommunication&& other) noexcept;
    SevenBitEncodedCommunication& operator=(SevenBitEncodedCommunication&& other) noexcept;

    bool writeMessage(const uint8_t* data, size_t length);
    bool readMessage(uint8_t* out, size_t maxOutLen, size_t& outLen);

  private:
    Communication& _inner;

    uint8_t* _txBuffer;
    uint8_t* _rxBuffer;
    size_t _txSize;
    size_t _rxSize;
    size_t _rxIndex;
};
