
Sketch for Arduino Nano 3.0 (CH340 - China) and board STM32F103C8T6 (Blue Pill)

mt7530d.ino: VLAN switch setup based on EcoNet/Mediatek MT7530D chip.

Copyright (C) 2017 McMCC <mcmcc_at_mail_ru>

For ARDUINO AVR:

Pin MDIO - D6 (Logic Level convertor 5v-to-3.3v pin MDIO on switch)
Pin MDC  - D5 (Logic Level convertor 5v-to-3.3v pin MDC on switch)
Pin GND  - D7 (only used as for fastening to the switch board)


For ARDUINO STM32:

No need Logic Level Convertor, use pin-to-pin conection and
pullup 10K resistors (between the pins and 3.3v from the switch board).

Pin MDIO - PB10 (and used as for fastening to the switch board)
Pin MDC  - PB11 (and used as for fastening to the switch board)
