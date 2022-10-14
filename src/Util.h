#pragma once

#include <stdint.h>
#include <Print.h>

class Uint64Print : public Printable
{
  public:
    Uint64Print(uint64_t v): _v(v) {}
    virtual size_t printTo(Print& s) const
    {
      if(_v == 0) return s.print(0);
      char rev[32];
      char *p = rev + 1;
      uint64_t num = _v;
      while (num > 0) {
        *p++ = '0' + (num % 10);
        num /= 10;
      }
      p--;
      // Print the number which is now in reverse
      size_t n = 0;
      while (p > rev) {
        n += s.print(*p--);
      }
      return n;
    }
  private:
    uint64_t _v;
};
