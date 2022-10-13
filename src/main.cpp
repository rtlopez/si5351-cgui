#include <Arduino.h>
#include <Wire.h>
#include <si5351.h>

Si5351 si5351;

uint64_t frequencies[3] = {0, 0, 0};

void setup()
{
  // Start serial and 
  Serial.begin(115200);


  // initialize the Si5351
  if(!si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0))
  {
    Serial.println(F("SI5351 not found on I2C bus! Check wiring and restart."));
    while(1);
  }

  Wire.setClock(400000ul);
  Serial.println(F("I2C speed 400 kHz."));

  // Set CLK0 to output 1 MHz
  si5351.set_freq(100000000ull, SI5351_CLK0);

  // Query a status update and wait a bit to let the Si5351 populate the status flags correctly.
  si5351.update_status();
  delay(500);
  Serial.println(F("Setup done."));
}

#define CMD_BUFF_SIZE 64u
#define CMD_ARGS_SIZE 8u

const char * DELIM = " \t";
size_t cmdIdx = 0;
char cmdBuff[CMD_BUFF_SIZE];
const char * args[8];

bool cmdIs(const char** args, const char * cmd)
{
  return strcmp(args[0], cmd) == 0;
}

bool cmdStatus(const char** args)
{
  if(!cmdIs(args, "status")) return false;
  return true;
}

bool cmdSetFreq(const char** args)
{
  if(!cmdIs(args, "freq_set")) return false;
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

bool cmdHelp(const char** args)
{
  if(args && !cmdIs(args, "help")) return false;
  Serial.println(F("commands: "));
  Serial.println(F("- help"));
  Serial.println(F("- freq_set <output_num(0-2)> <freq(8000-150000000)>"));
  return true;
}

void cmdUnknonwn()
{
  Serial.println(F("ERR: unknown command"));
  cmdHelp(nullptr);
}

void cmdProcess()
{
  if(!args[0]) return;

  bool processed = cmdStatus(args);
  processed |= cmdSetFreq(args);
  processed |= cmdHelp(args);

  if(!processed) cmdUnknonwn();
}

void cmdReset()
{
  for(size_t i = 0; i < CMD_ARGS_SIZE; ++i) {
    args[i] = nullptr;
  }
}

void cmdParse()
{
  Serial.println(cmdBuff);
  cmdReset();
  char * pch = strtok(cmdBuff, DELIM);
  size_t count = 0;
  while(pch)
  {
    args[count++] = pch;
    pch = strtok(NULL, DELIM);
  }
  cmdProcess();
}

void cmdRead(char c)
{
  // enter pressed, process command
  if(c == '\r' || c == '\n') {
    if(cmdIdx == 0) return; // empty buffer
    cmdBuff[cmdIdx] = 0;
    cmdIdx = 0;
    cmdParse();
    return;
  }

  // ignore too long command
  if(cmdIdx >= CMD_BUFF_SIZE - 1) return;

  // add to buffer
  cmdBuff[cmdIdx++] = c;
}

void loop()
{
  while(Serial.available()) {
    cmdRead(Serial.read());
  }

  unsigned long now = millis();
  static unsigned long nextStatus = 0;
  if(now > nextStatus) {
    nextStatus = now + 1000ul;
    // Read the Status Register and print it every few seconds
    si5351.update_status();
  }
}