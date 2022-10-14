#include <Arduino.h>
#include <Wire.h>
#include <si5351.h>
#include "Cli.h"

Si5351 si5351;
uint64_t frequencies[3] = {0, 0, 0};

bool cmdStatus(const char** args)
{
  if(!Cli::is(args, "status")) return false;
  return true;
}

bool cmdSetFreq(const char** args)
{
  if(!Cli::is(args, "freq_set")) return false;
  if(!args[1]) {
    Serial.println(F("ERR: no output specified"));
    return true;
  }
  if(!args[2]) {
    Serial.println(F("ERR: no frequency specified"));
    return true;
  }

  const int idx = String(args[1]).toInt();
  if(idx < 0 || idx > 2) {
    Serial.println(F("ERR: output must be 0, 1 or 2"));
    return true;
  }
  const long freq = String(args[2]).toInt();
  if(freq < 8000l || freq > 150000000l) {
    Serial.println(F("ERR: frequency out of range 8000-150000000"));
    return true;
  }
  Serial.print("freq_set ");
  Serial.print(idx);
  Serial.print(' ');
  Serial.print(freq);
  frequencies[idx] = freq * 100ull;
  si5351.set_freq(frequencies[idx], (si5351_clock)idx);
  Serial.print(' ');
  Serial.print((uint32_t)frequencies[idx]);
  Serial.println();
  return true;
}

bool cmdHelp()
{
  Serial.println(F("Available commands: "));
  Serial.println(F("- help"));
  Serial.println(F("- status"));
  Serial.println(F("- freq_set <output_num(0-2)> <freq(8000-150000000)>"));
  Serial.println(F("- freq_get <output_num(0-2)>"));
  return true;
}

bool cmdHelp(const char** args)
{
  if(!Cli::is(args, "help")) return false;
  return cmdHelp();
}

bool cmdUnknonwn(const char** args)
{
  Serial.print(F("ERR: Unknown command: \""));
  Serial.print(args[0]);
  Serial.println("\"");
  return cmdHelp();
}

Cli::func_t cliFunctions[] = {
  cmdHelp,
  cmdSetFreq,
  cmdStatus,
  nullptr // required null at the end
};

Cli cmd(cliFunctions, cmdUnknonwn);

void setup()
{
  // Start serial 
  Serial.begin(115200);

  // initialize the Si5351
  if(!si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0))
  {
    Serial.println(F("SI5351 not found on I2C bus! Check wiring and restart."));
    while(1);
  }

  // tune I2C, must be after Wire.begin() whic is called inside Si5351.init()
  Wire.setClock(400000ul);
  Serial.println(F("I2C speed 400 kHz."));

  // Set CLK0 to output 1 MHz
  si5351.set_freq(100000000ull, SI5351_CLK0);

  // Query a status update and wait a bit to let the Si5351 populate the status flags correctly.
  si5351.update_status();
  delay(500);
  Serial.println(F("Ready."));
}

void loop()
{
  while(Serial.available()) {
    cmd.parse(Serial.read());
  }

  unsigned long now = millis();
  static unsigned long nextStatus = 0;
  if(now > nextStatus) {
    nextStatus = now + 1000ul;
    // Read the Status Register and print it every few seconds
    si5351.update_status();
  }
}