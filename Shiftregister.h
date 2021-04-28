#pragma once

#include <SPI.h>


template<size_t CHAIN_LENGTH>
class Shiftregister {

  public:
    Shiftregister(int8_t mosi_pin, int8_t clock_pin, int8_t chip_select_pin) : chip_select_pin_(chip_select_pin) {
      SPI.begin(clock_pin, -1, mosi_pin, chip_select_pin);
      pinMode(chip_select_pin, OUTPUT);
    }

    void send(byte data[CHAIN_LENGTH]) {
      SPI.beginTransaction(SPISettings(clock_freq, MSBFIRST, SPI_MODE0));
      digitalWrite(chip_select_pin_, LOW); //pull SS slow to prep other end for transfer
      for (size_t i = 0; i < CHAIN_LENGTH; i++) {
        SPI.transfer(data[i]);
      }
      digitalWrite(chip_select_pin_, HIGH); //pull ss high to signify end of data transfer
      SPI.endTransaction();
    }

  private:
    int8_t chip_select_pin_;
    static const auto clock_freq = 1000000;

};
