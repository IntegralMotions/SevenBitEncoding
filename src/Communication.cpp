#include "Communication.h"

void Communication::write(const uint8_t* data, size_t size) {
    writeImpl(data, size);
}

size_t Communication::available() {
    return availableImpl();
}

size_t Communication::read(uint8_t* data, size_t size) {
    return readImpl(data, size);
}
