#pragma once

#include <EEPROM.h>

class Conf
{
  public:
    Conf() {}

    int begin()
    {
      EEPROM.begin();
      return load();
    }

    void save()
    {
      EEPROM.put(0, *this);
    }

    int load()
    {
      Conf t;
      EEPROM.get(0, t);
      if(magic != t.magic) return 1;
      *this = t;
      return 0;
    }

    uint16_t magic = 0xa55a;
    uint16_t i2cSpeed = 100;
    uint64_t freqs[3] = {0, 0, 0};
};
