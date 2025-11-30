#include "seven_bit_encoding.h"
#include <cstddef>
#include <cstdint>

namespace SevenBitEncoding {
    inline constexpr uint8_t LAST_SEVEN_BITS = 0x7F;
    inline constexpr uint8_t FIRST_BIT = 0x80;
    inline constexpr int ENCODING_SIZE = 7;
    inline constexpr int MAX_SHIFTS_FOR_VALUE = 32;

    size_t getEncodedSize(uint32_t value) {
        size_t size = 0;
        do {
            size++;
            value >>= ENCODING_SIZE;
        } while (value > 0);
        return size;
    }

    void encodeValue(uint32_t value, uint8_t* output) {
        size_t index = 0;
        do {
            uint8_t byte = value & LAST_SEVEN_BITS; // Take the lower 7 bits
            value >>= ENCODING_SIZE;
            if (value > 0) {
                byte |= FIRST_BIT; // Set the 8th bit if more bytes are needed
            }
            output[index++] = byte;
        } while (value > 0);
    }

    uint32_t decodeValue(const uint8_t* input, size_t inputSize, size_t& consumedBytes) {
        uint32_t length = 0;
        size_t shift = 0;
        consumedBytes = 0;

        for (size_t i = 0; i < inputSize; i++) {
            uint8_t byte = input[i];
            length |= (byte & LAST_SEVEN_BITS) << shift; // Extract 7 bits and shift into place
            consumedBytes++;

            if ((byte & FIRST_BIT) == 0) // Stop if the continuation bit is not set
            {
                break;
            }

            shift += ENCODING_SIZE;
            if (shift >= MAX_SHIFTS_FOR_VALUE) // Prevent overflow for invalid input
            {
                break;
            }
        }

        return length;
    }

    size_t getEncodedBufferSize(const size_t bufferLength) {
        return (bufferLength > 0) ? bufferLength + ((bufferLength - 1) / ENCODING_SIZE) + 1 : 1;
    }

    size_t encodeBuffer(const uint8_t* inputBuffer, const size_t inputLength, uint8_t* outputBuffer) {
        if (inputLength == 0) {
            return 0;
        }
        size_t outIndex = 0;
        uint8_t carry = 0;
        int carryBits = 0;
        for (size_t i = 0; i < inputLength; i++) {
            uint8_t current = inputBuffer[i];
            uint8_t septet = carry | (current >> (carryBits + 1));
            outputBuffer[outIndex++] = septet | FIRST_BIT;
            carryBits++;
            carry = (current & leftMask(carryBits)) << (ENCODING_SIZE - carryBits);
            if (carryBits == ENCODING_SIZE) {
                outputBuffer[outIndex++] = carry | FIRST_BIT;
                carry = 0;
                carryBits = 0;
            }
        }
        if (carryBits != 0) {
            uint8_t septet = carry;
            outputBuffer[outIndex++] = septet;
        }
        outputBuffer[outIndex - 1] &= LAST_SEVEN_BITS;
        return outIndex;
    }

    size_t decodeBuffer(const uint8_t* inputBuffer, size_t inputLength, uint8_t* outputBuffer, size_t outputLength) {
        if (inputBuffer == 0 || outputLength == 0) {
            return 0;
        }
        size_t decoded = 0;
        size_t encodedIndex = 0;
        int bitShiftIndex = 0;
        while (decoded < outputLength && encodedIndex + 1 < inputLength) {
            uint8_t currentByte = inputBuffer[encodedIndex] & LAST_SEVEN_BITS;
            uint8_t nextByte = inputBuffer[encodedIndex + 1] & LAST_SEVEN_BITS;
            int bits = bitShiftIndex + 1;
            uint8_t carry = (nextByte >> (ENCODING_SIZE - bits)) & ((1 << bits) - 1);
            uint8_t upperPart = currentByte & ((1 << (ENCODING_SIZE - bitShiftIndex)) - 1);
            uint8_t value = (upperPart << bits) | carry;
            outputBuffer[decoded++] = value;
            bitShiftIndex++;
            encodedIndex++;
            if (bitShiftIndex == ENCODING_SIZE) {
                encodedIndex++;
                bitShiftIndex = 0;
            }
        }
        return decoded;
    }

    bool isLastByte(const uint8_t byte) {
        return (byte & FIRST_BIT) == 0;
    }
} // namespace SevenBitEncoding