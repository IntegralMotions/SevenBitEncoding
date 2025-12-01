#include "IntegralCommunication/Communication.h"
#include "IntegralCommunication/SevenBitEncodedCommunication.h"
#include "IntegralCommunication/SevenBitEncoding.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <vector>

// ---------------------------
// Fake Communication for tests
// ---------------------------

class FakeCommunication : public Communication {
  public:
    const std::vector<uint8_t>& written() const {
        return _written;
    }

    // Test helper: push incoming encoded bytes that will be "read" by SevenBitEncodedCommunication.
    void pushIncoming(const std::vector<uint8_t>& data) {
        _incoming.insert(_incoming.end(), data.begin(), data.end());
    }

  private:
    void writeImpl(const uint8_t* data, size_t size) override {
        _written.insert(_written.end(), data, data + size);
    }

    size_t availableImpl() override {
        return _incoming.size();
    }

    size_t readImpl(uint8_t* data, size_t size) override {
        const size_t toRead = std::min(size, _incoming.size());
        if (toRead == 0) {
            return 0;
        }
        std::memcpy(data, _incoming.data(), toRead);
        _incoming.erase(_incoming.begin(), _incoming.begin() + static_cast<std::ptrdiff_t>(toRead));
        return toRead;
    }

    std::vector<uint8_t> _written;
    std::vector<uint8_t> _incoming;
};

// ---------------------------
// Tests
// ---------------------------

TEST(SevenBitEncodedCommunicationTests, WriteMessageEncodesAndForwards) {
    FakeCommunication fake;
    SevenBitEncodedCommunication comm(fake, 128, 128);

    const std::vector<uint8_t> payload = {0x01, 0x02, 0xFF, 0x10};

    // Compute expected encoded form using SevenBitEncoding directly
    const size_t needed = SevenBitEncoding::getEncodedBufferSize(payload.size());
    std::vector<uint8_t> expected(needed);
    const size_t encodedLen = SevenBitEncoding::encodeBuffer(payload.data(), payload.size(), expected.data());
    expected.resize(encodedLen);

    ASSERT_TRUE(comm.writeMessage(payload.data(), payload.size()));

    const auto& written = fake.written();
    ASSERT_EQ(written.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(written[i], expected[i]) << "byte " << i << " mismatch";
    }
}

TEST(SevenBitEncodedCommunicationTests, ReadMessageReturnsFalseWhenNoData) {
    FakeCommunication fake;
    SevenBitEncodedCommunication comm(fake, 128, 128);

    uint8_t out[32] = {};
    size_t outLen = 0;

    const bool result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_FALSE(result);
    EXPECT_EQ(outLen, 0u);
}

TEST(SevenBitEncodedCommunicationTests, ReadMessageNonBlockingPartialThenFull) {
    FakeCommunication fake;
    SevenBitEncodedCommunication comm(fake, 128, 128);

    const std::vector<uint8_t> payload = {0x10, 0x20, 0x30, 0x40};

    // Encode full message
    const size_t needed = SevenBitEncoding::getEncodedBufferSize(payload.size());
    std::vector<uint8_t> encoded(needed);
    const size_t encodedLen = SevenBitEncoding::encodeBuffer(payload.data(), payload.size(), encoded.data());
    encoded.resize(encodedLen);

    ASSERT_GT(encodedLen, 1u); // ensure we can create a partial sequence

    // Split encoded into partial and remainder
    const size_t partialLen = encodedLen - 1;
    std::vector<uint8_t> firstPart(encoded.begin(), encoded.begin() + partialLen);
    std::vector<uint8_t> secondPart(encoded.begin() + partialLen, encoded.end());

    // First call: only partial encoded message is available
    fake.pushIncoming(firstPart);

    uint8_t out[32] = {};
    size_t outLen = 0;
    bool result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_FALSE(result);
    EXPECT_EQ(outLen, 0u);

    // Second call: provide the last encoded byte, now full message should be available
    fake.pushIncoming(secondPart);

    result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_TRUE(result);
    EXPECT_EQ(outLen, payload.size());
    for (size_t i = 0; i < payload.size(); ++i) {
        EXPECT_EQ(out[i], payload[i]) << "byte " << i << " mismatch";
    }
}

TEST(SevenBitEncodedCommunicationTests, ReadTwoMessagesBackToBack) {
    FakeCommunication fake;
    SevenBitEncodedCommunication comm(fake, 128, 128);

    const std::vector<uint8_t> msg1 = {0x01, 0x02, 0x03};
    const std::vector<uint8_t> msg2 = {0xAA, 0xBB, 0xCC, 0xDD};

    // Encode both messages and concatenate their encoded bytes
    const size_t needed1 = SevenBitEncoding::getEncodedBufferSize(msg1.size());
    std::vector<uint8_t> enc1(needed1);
    size_t encLen1 = SevenBitEncoding::encodeBuffer(msg1.data(), msg1.size(), enc1.data());
    enc1.resize(encLen1);

    const size_t needed2 = SevenBitEncoding::getEncodedBufferSize(msg2.size());
    std::vector<uint8_t> enc2(needed2);
    size_t encLen2 = SevenBitEncoding::encodeBuffer(msg2.data(), msg2.size(), enc2.data());
    enc2.resize(encLen2);

    std::vector<uint8_t> combined;
    combined.reserve(enc1.size() + enc2.size());
    combined.insert(combined.end(), enc1.begin(), enc1.end());
    combined.insert(combined.end(), enc2.begin(), enc2.end());

    // Push both messages' encoded data at once
    fake.pushIncoming(combined);

    uint8_t out[32] = {};
    size_t outLen = 0;

    // First read: should get msg1
    bool result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_TRUE(result);
    EXPECT_EQ(outLen, msg1.size());
    for (size_t i = 0; i < msg1.size(); ++i) {
        EXPECT_EQ(out[i], msg1[i]) << "msg1 byte " << i << " mismatch";
    }

    // Second read: should get msg2
    std::memset(out, 0, sizeof(out));
    outLen = 0;
    result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_TRUE(result);
    EXPECT_EQ(outLen, msg2.size());
    for (size_t i = 0; i < msg2.size(); ++i) {
        EXPECT_EQ(out[i], msg2[i]) << "msg2 byte " << i << " mismatch";
    }

    // Third read: no more messages
    outLen = 0;
    result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_FALSE(result);
    EXPECT_EQ(outLen, 0u);
}
