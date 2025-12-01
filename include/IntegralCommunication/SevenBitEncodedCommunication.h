#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "Communication.h"
#include "SevenBitEncoding.h"

template <size_t TxSize, size_t RxSize> class SevenBitEncodedCommunication {
  public:
    explicit SevenBitEncodedCommunication(Communication& inner) : _inner(inner) {}

    bool writeMessage(const uint8_t* data, size_t length) {
        const size_t needed = SevenBitEncoding::getEncodedBufferSize(length);
        if (needed > TxSize) {
            return false; // tx buffer too small
        }

        // Encode into internal TX buffer
        const size_t encodedLen = SevenBitEncoding::encodeBuffer(data, length, _txBuffer.data());

        // Write encoded bytes to underlying communication
        _inner.write(_txBuffer.data(), encodedLen);
        return true;
    }

    bool readMessage(uint8_t* out, size_t maxOutLen, size_t& outLen) {
        outLen = 0;

        const size_t available = _inner.available();
        if (_rxIndex < RxSize && available != 0) {
            const size_t space = RxSize - _rxIndex;
            const size_t toRead = std::min(available, space);
            if (toRead == 0) {
                return false;
            }

            const size_t read = _inner.read(_rxBuffer.data() + _rxIndex, toRead);
            if (read == 0) {
                return false;
            }

            _rxIndex += read;
        }

        if (_rxIndex == 0) {
            return false;
        }

        size_t encodedLen = 0;
        bool found = false;

        for (size_t i = 0; i < _rxIndex; ++i) {
            if (SevenBitEncoding::isLastByte(_rxBuffer[i])) {
                encodedLen = i + 1;
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }

        const size_t decodedLen = SevenBitEncoding::decodeBuffer(_rxBuffer.data(), encodedLen, out, maxOutLen);

        if (decodedLen == 0 && encodedLen != 0) {
            const size_t remaining = _rxIndex - encodedLen;
            if (remaining > 0) {
                std::memmove(_rxBuffer.data(), _rxBuffer.data() + encodedLen, remaining);
            }
            _rxIndex = remaining;
            return false;
        }

        outLen = decodedLen;

        const size_t remaining = _rxIndex - encodedLen;
        if (remaining > 0) {
            std::memmove(_rxBuffer.data(), _rxBuffer.data() + encodedLen, remaining);
        }
        _rxIndex = remaining;

        return true;
    }

  private:
    Communication& _inner;

    std::array<uint8_t, TxSize> _txBuffer;
    std::array<uint8_t, RxSize> _rxBuffer;
    size_t _rxIndex = 0;
};
