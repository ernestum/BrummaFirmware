#pragma once

#include <SPI.h>


template<size_t NUM_BYTES>
class Shiftregister {

  public:
    Shiftregister() {
      SPI.begin();
    }

    void send(byte data[NUM_BYTES]) {
      SPI.beginTransaction(SPISettings(clock_freq, MSBFIRST, SPI_MODE0));
      for (size_t i = 0; i < NUM_BYTES; i++) {
        SPI.transfer(data[i]);
      }
      SPI.endTransaction();
    }

  private:
    static const auto clock_freq = 1000000;

};
