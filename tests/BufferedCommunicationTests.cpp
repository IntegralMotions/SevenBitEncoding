#include "BufferedCommunication.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <vector>

class TestBufferedCommunication : public BufferedCommunication {
  public:
    using BufferedCommunication::buffer;
    using BufferedCommunication::bufferIndex;
    using BufferedCommunication::bufferSize;

    TestBufferedCommunication(uint8_t* buffer, size_t bufferSize) : BufferedCommunication(buffer, bufferSize) {}

    const std::vector<uint8_t>& sink() const {
        return _sink;
    }

  private:
    size_t writeImpl(const uint8_t* data, size_t size) override {
        _sink.insert(_sink.end(), data, data + size);
        return size; // simulate full write
    }

    size_t availableImpl() override {
        return 0;
    }
    size_t readImpl(uint8_t*, size_t) override {
        return 0;
    }

    std::vector<uint8_t> _sink;
};

TEST(BufferedCommunicationTests, WriteDoesNotFlushWhenNotFull) {
    uint8_t buffer[8] = {};
    TestBufferedCommunication comm(buffer, sizeof(buffer));

    const uint8_t data[] = {1, 2, 3};
    comm.write(data, sizeof(data));

    EXPECT_EQ(comm.sink().size(), 0u);
    EXPECT_EQ(comm.bufferIndex(), sizeof(data));
    for (size_t i = 0; i < sizeof(data); ++i) {
        EXPECT_EQ(comm.buffer()[i], data[i]);
    }
}

TEST(BufferedCommunicationTests, WriteFlushesWhenBufferFullAndKeepsRemainderInBuffer) {
    uint8_t buffer[4] = {};
    TestBufferedCommunication comm(buffer, sizeof(buffer));

    const uint8_t data[] = {10, 20, 30, 40, 50, 60};
    comm.write(data, sizeof(data));

    const auto& sink = comm.sink();
    ASSERT_EQ(sink.size(), 4u);
    EXPECT_EQ(sink[0], 10);
    EXPECT_EQ(sink[1], 20);
    EXPECT_EQ(sink[2], 30);
    EXPECT_EQ(sink[3], 40);

    EXPECT_EQ(comm.bufferIndex(), 2u);
    EXPECT_EQ(comm.buffer()[0], 50);
    EXPECT_EQ(comm.buffer()[1], 60);
}

class TestBufferedCommunicationPartial : public BufferedCommunication {
  public:
    using BufferedCommunication::bufferIndex;

    TestBufferedCommunicationPartial(uint8_t* buffer, size_t bufferSize, size_t maxPerCall)
        : BufferedCommunication(buffer, bufferSize), _maxPerCall(maxPerCall) {}

    const std::vector<uint8_t>& sink() const {
        return _sink;
    }

  private:
    size_t writeImpl(const uint8_t* data, size_t size) override {
        const size_t toWrite = std::min(size, _maxPerCall);
        _sink.insert(_sink.end(), data, data + toWrite);
        return toWrite;
    }

    size_t availableImpl() override {
        return 0;
    }
    size_t readImpl(uint8_t*, size_t) override {
        return 0;
    }

    std::vector<uint8_t> _sink;
    size_t _maxPerCall;
};

TEST(BufferedCommunicationTests, FlushHandlesPartialWritesCorrectly) {
    uint8_t buffer[8] = {};
    TestBufferedCommunicationPartial comm(buffer, sizeof(buffer), 3);

    const uint8_t data[] = {1, 2, 3, 4, 5};
    comm.write(data, sizeof(data));

    comm.flush();

    const auto& sink = comm.sink();
    ASSERT_EQ(sink.size(), sizeof(data));
    for (size_t i = 0; i < sizeof(data); ++i) {
        EXPECT_EQ(sink[i], data[i]);
    }
    EXPECT_EQ(comm.bufferIndex(), 0u);
}
