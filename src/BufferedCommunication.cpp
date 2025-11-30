#include "BufferedCommunication.h"

#include <cstring>

BufferedCommunication::BufferedCommunication(uint8_t* buffer, size_t bufferSize)
    : _buffer(buffer), _bufferSize(bufferSize), _bufferIndex(0) {}

BufferedCommunication::~BufferedCommunication() = default;

void BufferedCommunication::write(const uint8_t* data, size_t dataSize) {
    while (dataSize > 0) {
        size_t space = _bufferSize - _bufferIndex;

        if (space == 0) {
            flush();
            space = _bufferSize - _bufferIndex;
            if (space == 0) {
                // underlying writeImpl can't make space
                return;
            }
        }

        const size_t toCopy = (dataSize < space) ? dataSize : space;
        std::memcpy(_buffer + _bufferIndex, data, toCopy);
        _bufferIndex += toCopy;
        data += toCopy;
        dataSize -= toCopy;
    }
}

void BufferedCommunication::flush() {
    while (_bufferIndex > 0) {
        const size_t written = writeImpl(_buffer, _bufferIndex);
        if (written == 0) {
            // can't push anything out, stop to avoid infinite loop
            return;
        }

        if (written < _bufferIndex) {
            std::memmove(_buffer, _buffer + written, _bufferIndex - written);
        }
        _bufferIndex -= written;
    }
}

size_t BufferedCommunication::available() {
    return availableImpl();
}

size_t BufferedCommunication::read(uint8_t* data, size_t dataSize) {
    return readImpl(data, dataSize);
}

uint8_t* BufferedCommunication::buffer() noexcept {
    return _buffer;
}

const uint8_t* BufferedCommunication::buffer() const noexcept {
    return _buffer;
}

size_t BufferedCommunication::bufferIndex() const noexcept {
    return _bufferIndex;
}

size_t BufferedCommunication::bufferSize() const noexcept {
    return _bufferSize;
}
