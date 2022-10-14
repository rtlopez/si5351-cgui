#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <si5351.h>
#include "Conf.h"
#include "Cli.h"
#include "Util.h"

Conf conf;
Si5351 si5351;

void applySettings()
{
  Wire.setClock(conf.i2cSpeed * 1000ul);
  for(size_t i = 0; i < 3; i++) {
    const uint64_t freq = conf.freqs[i];
    if(freq) {
      si5351.set_freq(freq, (si5351_clock)i);
    } else {
      si5351.output_enable((si5351_clock)i, 0);
    }
  }
}

bool cmdSave(const char** args)
{
  if(!Cli::is(args, "save")) return false;
  conf.save();
  Serial.println(F("OK"));
  return true;
}

bool cmdLoad(const char** args)
{
  if(!Cli::is(args, "load")) return false;
  int code = conf.load();
  if(code != 0) {
    Serial.print(F("ERR: EEPROM corrupted: "));
    Serial.println(code);
    return true;
  }
  applySettings();
  Serial.println(F("OK"));
  return true;
}

bool cmdStatus(const char** args)
{
  if(!Cli::is(args, "status")) return false;
  for(size_t i = 0; i < 3; i++) {
    Serial.print(i);
    Serial.print(' ');
    Serial.print(Uint64Print(conf.freqs[i]));
    Serial.println();
  }
  return true;
}

bool cmdI2CSpeed(const char** args)
{
  if(!Cli::is(args, "i2c_speed")) return false;
  if(!args[1]) {
    Serial.println(conf.i2cSpeed);
    return true;
  }

  const int speed = String(args[1]).toInt();
  if(speed < 50 || speed > 400) {
    Serial.println(F("ERR: i2c speed out of range 50-400 kHz"));
    return true;
  }

  conf.i2cSpeed = speed;
  applySettings();

  return true;
}

bool cmdFreq(const char** args)
{
  if(!Cli::is(args, "freq")) return false;
  if(!args[1]) {
    Serial.println(F("ERR: no output specified"));
    return true;
  }
  const int idx = String(args[1]).toInt();
  if(idx < 0 || idx > 2) {
    Serial.println(F("ERR: output must be 0, 1 or 2"));
    return true;
  }

  // read freq
  if(!args[2]) {
    Serial.println((uint32_t)conf.freqs[idx]);
    return true;
  }

  // write freq
  const long freq = String(args[2]).toInt();
  if(freq != 0 && (freq < 8000l || freq > 150000000l)) {
    Serial.println(F("ERR: frequency out of range 8000-150000000"));
    return true;
  }

  conf.freqs[idx] = freq * 100ull;

  Serial.print("freq ");
  Serial.print(idx);
  Serial.print(' ');
  Serial.print(freq);
  Serial.print(F(" > "));
  Serial.print(Uint64Print(conf.freqs[idx]));
  Serial.println();

  applySettings();

  return true;
}

bool cmdHelp()
{
  Serial.println(F("Available commands: "));
  Serial.println(F("- help"));
  Serial.println(F("- save"));
  Serial.println(F("- load"));
  Serial.println(F("- status"));
  Serial.println(F("- i2c_speed <speed(50-400)>"));
  Serial.println(F("- freq <output_num(0-2)> [<freq(8000-150000000)>]"));
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
  cmdLoad,
  cmdSave,
  cmdFreq,
  cmdStatus,
  cmdI2CSpeed,
  nullptr // required null at the end
};

Cli cmd(cliFunctions, cmdUnknonwn);

void setup()
{
  // Start serial 
  Serial.begin(115200);

  // load configuration
  int code = conf.begin();
  if(code != 0) {
    Serial.print(F("EEPROM corrupted, load defauls, code: "));
    Serial.println(code);
  }

  // initialize the Si5351
  if(!si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0)) {
    Serial.println(F("ERR: SI5351 not found on I2C bus! Check wiring."));
    while(1);
  }

  // setup clock generator outputs
  applySettings();

  // Query a status update and wait a bit to let the Si5351 populate the status flags correctly.
  si5351.update_status();
  delay(500);
}

void loop()
{
  // process commands
  while(Serial.available()) {
    cmd.parse(Serial.read());
  }

  // reserved for future use
  unsigned long now = millis();
  static unsigned long nextStatus = 0;
  if(now > nextStatus) {
    nextStatus = now + 1000ul;
    // Read the Status Register every second
    si5351.update_status();
  }
}