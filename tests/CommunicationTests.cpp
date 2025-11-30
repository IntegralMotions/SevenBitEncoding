#include "Communication.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

class TestCommunication : public Communication {
  public:
    const std::vector<uint8_t>& written() const {
        return _written;
    }

    void setReadData(const std::vector<uint8_t>& data) {
        _readData = data;
    }

  private:
    void writeImpl(const uint8_t* data, size_t size) override {
        _written.insert(_written.end(), data, data + size);
    }

    size_t availableImpl() override {
        return _readData.size();
    }

    size_t readImpl(uint8_t* data, size_t size) override {
        const size_t toRead = std::min(size, _readData.size());
        std::memcpy(data, _readData.data(), toRead);
        _readData.erase(_readData.begin(), _readData.begin() + static_cast<std::ptrdiff_t>(toRead));
        return toRead;
    }

    std::vector<uint8_t> _written;
    std::vector<uint8_t> _readData;
};

TEST(CommunicationTests, WriteForwardsToImpl) {
    TestCommunication comm;
    const uint8_t data[] = {1, 2, 3, 4, 5};

    comm.write(data, sizeof(data));

    const auto& written = comm.written();
    ASSERT_EQ(written.size(), sizeof(data));
    for (size_t i = 0; i < sizeof(data); ++i) {
        EXPECT_EQ(written[i], data[i]);
    }
}

TEST(CommunicationTests, ReadAndAvailable) {
    TestCommunication comm;

    std::vector<uint8_t> input = {10, 20, 30, 40};
    comm.setReadData(input);

    EXPECT_EQ(comm.available(), input.size());

    uint8_t out[4] = {};
    const size_t read = comm.read(out, sizeof(out));
    EXPECT_EQ(read, input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        EXPECT_EQ(out[i], input[i]);
    }

    EXPECT_EQ(comm.available(), 0u);
}
