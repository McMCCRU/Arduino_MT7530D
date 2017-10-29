
Sketch for Arduino Nano 3.0 (CH340 - China) and board STM32F103C8T6 (Blue Pill)

mt7530.ino: VLAN switch setup based on EcoNet/Mediatek MT7530D chip.

Copyright (C) 2017 McMCC <mcmcc_at_mail_ru>

For ARDUINO AVR:

Pin MDIO - D6 (convertor 5v-to-3.3v pin MDIO on switch)
Pin MDC  - D5 (convertor 5v-to-3.3v pin MDC on switch)
Pin GND  - D7 (only used as for fastening to the switch board)

            Logic Level Convertor 5v-to-3.3v

          +3.3V                          +5V   SOT-23
            o------*--------       -------o    D __
                   |       |       |             ||
                   |  R1   |       |  R2     ----------
                  --- 10K  |      --- 10K    | BSS138 |
                  | |      |      | |        ----------
                  | |   Q1 |G     | |         ||    ||
                  ---   -------   ---       G --    -- S
       MDC,MDIO    |    -  ^  -    |
(MT7530D TTL 3.3v) |    |  |  |    |     D5,D6(Arduino TTL 5v)
            o------*----*---  *----*------o
                       S|     |D  Q1 - MOSFET N-Channel
                        |_|\|_|     BSS138(diode built-in)
                          |/|       [ or 2N7000/2N7002 ]


For ARDUINO STM32:

No need Logic Level Convertor, use pin-to-pin conection and
pullup 10K resistors (between the pins and 3.3v from the switch board).

Pin MDIO - PB10 (and used as for fastening to the switch board)
Pin MDC  - PB11 (and used as for fastening to the switch board)
