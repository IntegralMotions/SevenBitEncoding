#pragma once

#include <cstddef>
#include <cstdint>

class Communication {
public:
  Communication();
  virtual ~Communication();

  virtual void write(uint8_t *data, const uint8_t data_size) = 0;
  virtual size_t read(uint8_t *data, const uint8_t data_size) = 0;
};

