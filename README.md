# SI5351 Clock Generator User Interface

## Platforms

 - Atmega 328p (Arduino Pro Mini or Arduino Nano boards, or compatible)

## Serial Port CLI commands

```
help
Available commands: 
- help
- save
- load
- status
- i2c_speed <speed(50-400)>
- freq <output_num(0-2)> [<freq(8000-150000000)>]
```

## Wiring

## Atmega328p Boards

 - A4 <-> I2C SDA
 - A5 <-> I2C SCL
