/*
 * Sketch for Arduino Nano 3.0 (CH340 - China) and board STM32F103C8T6 (Blue Pill)
 *
 * mt7530d.ino: VLAN switch setup based on EcoNet/Mediatek MT7530D chip.
 *
 * Copyright (C) 2017 McMCC <mcmcc_at_mail_ru>
 *
 * For ARDUINO AVR:
 *
 * Pin MDIO - D6 (convertor 5v-to-3.3v pin MDIO on switch)
 * Pin MDC  - D5 (convertor 5v-to-3.3v pin MDC on switch)
 * Pin GND  - D7 (only used as for fastening to the switch board)
 *
 *             Logic Level Convertor 5v-to-3.3v
 *
 *           +3.3V                          +5V   SOT-23
 *             o------*--------       -------o    D __
 *                    |       |       |             ||
 *                    |  R1   |       |  R2     ----------
 *                   --- 10K  |      --- 10K    | BSS138 |
 *                   | |      |      | |        ----------
 *                   | |   Q1 |G     | |         ||    ||
 *                   ---   -------   ---       G --    -- S
 *        MDC,MDIO    |    -  ^  -    |
 * (MT7530D TTL 3.3v) |    |  |  |    |     D5,D6(Arduino TTL 5v)
 *             o------*----*---  *----*------o
 *                        S|     |D  Q1 - MOSFET N-Channel
 *                         |_|\|_|     BSS138(diode built-in)
 *                           |/|       [ or 2N7000/2N7002 ]
 *
 *
 * For ARDUINO STM32:
 *
 * No need Logic Level Convertor, use pin-to-pin conection and
 * pullup 10K resistors (between the pins and 3.3v from the switch board).
 *
 * Pin MDIO - PB10 (and used as for fastening to the switch board)
 * Pin MDC  - PB11 (and used as for fastening to the switch board)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if !defined(__AVR__) /* if STM32 Arduino */

#define PIN_MDIO				PB10
#define PIN_MDC					PB11

#include <inttypes.h>
#include <EEPROM.h>

struct EERef {

	EERef( const int index )
			: index( index )	{}

	/* Access/read members. */
	uint8_t operator*() const		{ return EEPROM.read( (uint16_t) index ); }
	operator const uint8_t() const		{ return **this; }

	/* Assignment/write members. */
	EERef &operator=( const EERef &ref )	{ return *this = *ref; }
	EERef &operator=( uint8_t in )		{ return EEPROM.write( (uint16_t) index, in ), *this;  }
	EERef &operator +=( uint8_t in )	{ return *this = **this + in; }
	EERef &operator -=( uint8_t in )	{ return *this = **this - in; }
	EERef &operator *=( uint8_t in )	{ return *this = **this * in; }
	EERef &operator /=( uint8_t in )	{ return *this = **this / in; }
	EERef &operator ^=( uint8_t in )	{ return *this = **this ^ in; }
	EERef &operator %=( uint8_t in )	{ return *this = **this % in; }
	EERef &operator &=( uint8_t in )	{ return *this = **this & in; }
	EERef &operator |=( uint8_t in )	{ return *this = **this | in; }
	EERef &operator <<=( uint8_t in )	{ return *this = **this << in; }
	EERef &operator >>=( uint8_t in )	{ return *this = **this >> in; }

	EERef &update( uint8_t in )		{ return  in != *this ? *this = in : *this; }

	/* Prefix increment/decrement */
	EERef& operator++()			{ return *this += 1; }
	EERef& operator--()			{ return *this -= 1; }

	/* Postfix increment/decrement */
	uint8_t operator++ (int) {
		uint8_t ret = **this;
		return ++(*this), ret;
	}

	uint8_t operator-- (int){
		uint8_t ret = **this;
		return --(*this), ret;
	}

	int index; /* Index of current EEPROM cell. */
};

struct EEPtr {

	EEPtr( const int index )
			: index( index )	{}

	operator const int() const		{ return index; }
	EEPtr &operator=( int in )		{ return index = in, *this; }

	/* Iterator functionality. */
	bool operator!=( const EEPtr &ptr )	{ return index != ptr.index; }
	EERef operator*()			{ return index; }

	/* Prefix & Postfix increment/decrement */
	EEPtr& operator++()			{ return ++index, *this; }
	EEPtr& operator--()			{ return --index, *this; }
	EEPtr operator++ (int)			{ return index++; }
	EEPtr operator-- (int)			{ return index--; }

	int index; /* Index of current EEPROM cell. */
};

struct EEPROMClass_EMU {

	/* Basic user access methods. */
	EERef operator[]( const int idx )	{ return idx; }
	uint8_t read( int idx )			{ return EERef( idx ); }
	void write( int idx, uint8_t val )	{ (EERef( idx )) = val; }
	void update( int idx, uint8_t val )	{ EERef( idx ).update( val ); }

	/* Functionality to 'get' and 'put' objects to and from EEPROM. */
	template< typename T > T &get( int idx, T &t ) {
		EEPtr e = idx;
		uint8_t *ptr = (uint8_t*) &t;
		for( int count = sizeof(T) ; count ; --count, ++e )  *ptr++ = *e;
		return t;
	}

	template< typename T > const T &put( int idx, const T &t ) {
		EEPtr e = idx;
		const uint8_t *ptr = (const uint8_t*) &t;
		for( int count = sizeof(T) ; count ; --count, ++e )  (*e).update( *ptr++ );
		return t;
	}
};

static EEPROMClass_EMU EEPROM_EMU;

#define EEPROM_get(a, b)			EEPROM_EMU.get(a, b)
#define EEPROM_put(a, b)			EEPROM_EMU.put(a, b)
#define BOARD					"_STM32"
#define USEC					1

#else /* if AVR Arduino */

#include <EEPROM.h>

#define PIN_GND					7 /* Not use */
#define PIN_MDIO				6
#define PIN_MDC					5
#define EEPROM_get(a, b)			EEPROM.get(a, b)
#define EEPROM_put(a, b)			EEPROM.put(a, b)
#define BOARD					"_AVR"
#define USEC					1

#endif /* End support board */

#define VERFW					"v.1.0.02_MT7530DU"
#define MAGIC_EEPROM_START			0x7530

#define MAX_VLAN_GROUP				8
#define MAX_PORTS				5

/* If Port1 = P0, Port2 = P1...etc, please disable this define */
#define PORTS_INVERSION				1

#ifdef PORTS_INVERSION
#define VERSION					VERFW BOARD "_INV"
#define PORT_INV(x)				(4 - x)
#else
#define VERSION					VERFW BOARD
#define PORT_INV(x)				(x)
#endif

struct eeprom_vlan_record {
	uint16_t vid;
	uint8_t ports_mask;
	uint8_t prio;
};

/* Magic EEPROM start + tag/untag masks group + idx + vlan record structure * 8 */
#define CFG_SIZE (2 + 2 + 1 + (sizeof(eeprom_vlan_record) * 8))

String readString;

void smi_out_bit(int32_t bit)
{
	digitalWrite(PIN_MDC, LOW);
	if (bit == 0)
		digitalWrite(PIN_MDIO, LOW);
	else
		digitalWrite(PIN_MDIO, HIGH);
	delayMicroseconds(USEC);
	digitalWrite(PIN_MDC, HIGH);
	delayMicroseconds(USEC);
}

uint32_t smi_in_bit()
{
	unsigned int res = 0;

	digitalWrite(PIN_MDC, LOW);
	delayMicroseconds(USEC);
	res = digitalRead(PIN_MDIO);
	digitalWrite(PIN_MDC, HIGH);
	delayMicroseconds(USEC);

	return res == HIGH ? 1 : 0;
}

uint32_t mii_mgr_read(uint32_t phyaddr, uint32_t regaddr)
{
	int32_t i = 0;
	uint32_t res = 0;

	pinMode(PIN_MDC, OUTPUT);
	smi_in_bit();
	smi_in_bit(); /* IDLE */
	pinMode(PIN_MDIO, OUTPUT);
	digitalWrite(PIN_MDC, HIGH);
	smi_out_bit(0); /* START */
	smi_out_bit(1);
	smi_out_bit(1); /* READ */
	smi_out_bit(0);

	for (i = 4; i >= 0; i--)
		smi_out_bit((phyaddr >> i) & 0x1);
	for (i = 4; i >= 0; i--)
		smi_out_bit((regaddr >> i) & 0x1);

	pinMode(PIN_MDIO, INPUT);
	digitalWrite(PIN_MDIO, HIGH); /* Pullup */
	smi_in_bit(); /* Z-state */
	smi_in_bit();
	for (i = 15; i >= 0; i--)
		res |= (smi_in_bit() << i);
	return res;
}

void mii_mgr_write(uint32_t phyaddr, uint32_t regaddr, uint32_t value)
{
	int32_t i = 0;
	uint32_t res = 0;
	pinMode(PIN_MDC, OUTPUT);
	smi_in_bit();
	smi_in_bit(); /* IDLE */
	pinMode(PIN_MDIO, OUTPUT);
	digitalWrite(PIN_MDC, HIGH);
	smi_out_bit(0); /* START */
	smi_out_bit(1);
	smi_out_bit(0); /* WRITE */
	smi_out_bit(1);

	for (i = 4; i >= 0; i--)
		smi_out_bit((phyaddr >> i) & 0x1);
	for (i = 4; i >= 0; i--)
		smi_out_bit((regaddr >> i) & 0x1);
	smi_out_bit(1); /* TA */
	smi_out_bit(0);

	for (i = 15; i >= 0; i--)
		smi_out_bit((value >> i) & 0x1);

	pinMode(PIN_MDIO, INPUT);
	digitalWrite(PIN_MDIO, HIGH);
	smi_in_bit();
	smi_in_bit(); /* IDLE */
}

uint32_t gswPbusRead(uint32_t pbus_addr)
{
	uint32_t pbus_data;

	uint32_t phyaddr;
	uint32_t reg;
	uint32_t value;

	phyaddr = 31;
	/* 1. Write high-bit page address */
	reg = 31;
	value = (pbus_addr >> 6);
	mii_mgr_write(phyaddr, reg, value);

	/* 2. Read low DWord */
	reg = (pbus_addr >> 2) & 0x000f;
	value = mii_mgr_read(phyaddr, reg);
	pbus_data = value;

	/* 3. Read high DWord */
	reg = 16;
	value = mii_mgr_read(phyaddr, reg);

	pbus_data = (pbus_data) | (value << 16);

	return pbus_data;
}

void gswPbusWrite(uint32_t pbus_addr, uint32_t pbus_data)
{
	uint32_t phyaddr;
	uint32_t reg;
	uint32_t value;

	phyaddr = 31;

	/* 1. Write high-bit page address */
	reg = 31;
	value = (pbus_addr >> 6);
	mii_mgr_write(phyaddr, reg, value);

	/* 2. Write low DWord */
	reg = (pbus_addr >> 2) & 0x000f;
	value = pbus_data & 0xffff;
	mii_mgr_write(phyaddr, reg, value);

	/* 3. Write high DWord */
	reg = 16;
	value = (pbus_data >> 16) & 0xffff;
	mii_mgr_write(phyaddr, reg, value);
}

void mt7530_link_status()
{
	uint32_t val, lr_speed;

	Serial.println(F("\n------ Link Status -------"));
	for (int i = 0; i < MAX_PORTS; i++) {
		val = gswPbusRead(0x3008 + (0x100 * PORT_INV(i)));

		Serial.print(F("Port"));
		Serial.print(i + 1, DEC);
		Serial.print(F(": "));
		if (val & 1) {

			lr_speed = (val >> 2) & 3;
			if (lr_speed == 0) {
				Serial.print(F("10M "));
			}
			else if (lr_speed == 1) {
				Serial.print(F("100M "));
			}
			else if ((lr_speed == 2)|| (lr_speed == 3)) {
				Serial.print(F("1000M "));
			}
			if ((val >> 1) & 1) {
				Serial.println(F("FD"));
			} else {
				Serial.println(F("HD"));
			}
		} else {
			Serial.println(F("Link DOWN"));
		}
	}
	Serial.println(F("---- End Link Status ------"));
}

uint32_t mt7530_vlan_table_busy()
{
	int i;
	uint32_t value = 0;

	for (i = 0; i < 20; i++) {
		value = gswPbusRead(0x90);
		if ((value & 0x80000000) == 0 ) /* table busy */
			break;
		delay(50);
	}

	if(i == 20) {
		Serial.println(F("...timeout!"));
	}

	return value;
}

void mt7530_port_tagget(int port)
{
	gswPbusWrite(0x2004 + (port * 0x100), 0x20ff0003);
	gswPbusWrite(0x2010 + (port * 0x100), 0x81000000);
}

void mt7530_port_untagget(int port)
{
	gswPbusWrite(0x2004 + (port * 0x100), 0xff0003);
	gswPbusWrite(0x2010 + (port * 0x100), 0x810000c0);
}

void mt7530_port_set_vid(int port, uint32_t vid)
{
	gswPbusWrite(0x2014 + (port * 0x100), 0x10000 + (vid & 0xfff));
}

void mt7530_clear_all_vlan_table()
{
	for (int i = 0; i < MAX_PORTS; i++) {
		gswPbusWrite(0x2004 + (i * 0x100), 0xff0000);
		gswPbusWrite(0x2010 + (i * 0x100), 0xc0);
	}
	gswPbusWrite(0x80, 0x8002);
	delay(1000);
}

void mt7530_add_vlan_table(uint8_t mask_ports, uint32_t vid, uint8_t prio)
{
	uint32_t value;

	mt7530_vlan_table_busy();
	value = (1UL << 31) | (0 << 12) | vid; /* 0: read specific VLAN entry */
	gswPbusWrite(0x90, value);

	value = mt7530_vlan_table_busy();
	if (value & (1UL << 16)) {
		Serial.println(F("Index is out of valid index."));
		return;
	}

	value = ((unsigned long)mask_ports << 16); /* Set vlan member */
	value |= (1UL << 30); /* IVL = 1 */
	value |= (((unsigned long)prio & 0x7) << 24); /* Priority 0~7, 0 - not used priority */
	value |= ((vid & 0xfff) << 4); /* VID */
	value |= 1; /* Valid */

	gswPbusWrite(0x94, value);

	value = (1UL << 31) | (1 << 12) | vid; /* 1: write specific VLAN entry */
	gswPbusWrite(0x90, value);
	mt7530_vlan_table_busy();
}

void mt7530_prio_init()
{
	uint32_t value;
#if 0
	/* Disable Global Flow Control */
	value = gswPbusRead(0x1fe0);
	value &= ~(1UL << 31);
	gswPbusWrite(0x1fe0, value);
#endif
	/* Set only Priority TAG User Priority Weight */
	gswPbusWrite(0x44, 7 << 8); /* Set 7 is max */

	/* Set only Priority TAG User Priority Egress Mapping */
	value = gswPbusRead(0x48);
	value &= ~((7 << 8) | (7 << 11) | (7UL << 24) | (7UL << 27));
	value |=  ((0 << 11) | (1UL << 27)); /* Set Prio0 and Prio1 queue 0 and 1 */
	gswPbusWrite(0x48, value);

	value = gswPbusRead(0x4c);
	value &= ~((7 << 8) | (7 << 11) | (7UL << 24) | (7UL << 27));
	value |=  ((2 << 11) | (3UL << 27)); /* Set Prio2 and Prio3 queue 2 and 3 */
	gswPbusWrite(0x4c, value);

	value = gswPbusRead(0x50);
	value &= ~((7 << 8) | (7 << 11) | (7UL << 24) | (7UL << 27));
	value |=  ((4 << 11) | (5UL << 27)); /* Set Prio4 and Prio5 queue 4 and 5 */
	gswPbusWrite(0x50, value);

	value = gswPbusRead(0x54);
	value &= ~((7 << 8) | (7 << 11) | (7UL << 24) | (7UL << 27));
	value |=  ((6 << 11) | (7UL << 27)); /* Set Prio6 and Prio7 queue 6 and 7 */
	gswPbusWrite(0x54, value);
}

#if 0
void mt7530_set_vlan_example()
{
	int i;

	mt7530_port_tagget(0); /* Set P0 tagged */
	for (i = 1; i < MAX_PORTS; i++) /* Set P1..P4 untagget */
		mt7530_port_untagget(i);

	/* Set PVID */
	mt7530_port_set_vid(1, 200); /* P1 PVID=200 */
	for (i = 2; i < MAX_PORTS; i++)
		mt7530_port_set_vid(i, 100); /* P2..P4 PVID=100 */

	/* Set VLAN */
	/* 11101 - P0,P2..P4 VLAN=100 */
	mt7530_add_vlan_table(0x1d, 100, 0);
	/* 00011 - P0 and P1 VLAN=200 */
	mt7530_add_vlan_table(0x3, 200, 0);
}
#endif

void mt7530_vlan_table_dump()
{
	uint32_t i, vid, value;
	int j, empty = 1;
	char fvid[4];

	Serial.println(F("\n---- Start table list -------"));
	for (i = 0; i < 4095; i++) {
		mt7530_vlan_table_busy();
		value = (1UL << 31) | (0 << 12) | i; /* 0: read specific VLAN entry */
		gswPbusWrite(0x90, value);

		value = mt7530_vlan_table_busy();
		if (value & (1UL << 16)) {
			Serial.println(F("Index is out of valid index."));
			return;
		}

		value = gswPbusRead(0x94);

		if ((value & 0x01) != 0) {
			if(empty)
				Serial.println(F(" VID    Ports    Priority"));
			empty = 0;
			sprintf(fvid, "%04d  ", i);
			Serial.print(fvid);
			for (j = 0; j < MAX_PORTS; j++) {
				if((value & (1UL << (16 + j))) != 0) {
					Serial.print(PORT_INV(j) + 1, DEC);
					Serial.print(F(" "));
				} else
					Serial.print(F("- "));
			}
			Serial.print(F("    "));
			Serial.print(((unsigned long)value >> 24) & 0x7, DEC); /* Prio */
			Serial.println(F(""));
		}
	}
	if(empty)
		Serial.println(F("Table is empty!"));
	Serial.println(F("---- End table list ---------"));
}

void mt7530_phy_setting()
{
	uint32_t regValue;

	for (int i = 0; i < MAX_PORTS; i++) {
		/* Disable EEE */
		mii_mgr_write(i, 13, 0x07);
		mii_mgr_write(i, 14, 0x3c);
		mii_mgr_write(i, 13, 0x4007);
		mii_mgr_write(i, 14, 0x0);

		/* Increase SlvDPSready time */
		mii_mgr_write(i, 31, 0x52b5);
		mii_mgr_write(i, 16, 0xafae);
		mii_mgr_write(i, 18, 0x2f);
		mii_mgr_write(i, 16, 0x8fae);

		/* Incease post_update_timer */
		mii_mgr_write(i, 31, 0x3);
		mii_mgr_write(i, 17, 0x4b);

		/* Adjust 100_mse_threshold */
		mii_mgr_write(i, 13, 0x1e);
		mii_mgr_write(i, 14, 0x123);
		mii_mgr_write(i, 13, 0x401e);
		mii_mgr_write(i, 14, 0xffff);

		/* Disable mcc */
		mii_mgr_write(i, 13, 0x1e);
		mii_mgr_write(i, 14, 0xa6);
		mii_mgr_write(i, 13, 0x401e);
		mii_mgr_write(i, 14, 0x300);

		/* Enable HW auto downshift */
		mii_mgr_write(i, 31, 0x1);
		regValue = mii_mgr_read(i, 20);
		regValue |= (1 << 4);
		mii_mgr_write(i, 20, regValue);

		/* Increase 10M mode RX gain for long cable */
		mii_mgr_write(i, 31, 0x52b5);
		mii_mgr_write(i, 16, 0xaf92);
		mii_mgr_write(i, 17, 0x8689);
		mii_mgr_write(i, 16, 0x8fae);
	}
}

void mt7530_init()
{
	int i;
	uint32_t regValue;

	Serial.print(F("\nInit MT7530D switch..."));

	/* Down all MAC */
	for (i = 0; i < MAX_PORTS; i++)
		gswPbusWrite(0x3000 + 0x100 * i, 0x8000);

	/* Soft reset switch */
	gswPbusWrite(0x7000, 0x0003);

	/* Delay 100ms */
	delay(100);

	/* Configure MT7530 HW-TRAP and PHY patch setting */
	for (i = 0; i < 2; i++) {
		regValue = gswPbusRead(0x7804);
		regValue |=  (1UL << 16);	/* Change HW-TRAP */
		regValue |=  (1UL << 24);	/* Standalone Switch */
		regValue &= ~(1 << 15);		/* Switch clock = 500MHz */
		regValue &= ~(1 << 5);		/* PHY direct access mode */
		gswPbusWrite(0x7804, regValue);

		/* Delay 100ms */
		delay(100);

		/* PHY patch setting */
		mt7530_phy_setting();
	}

	/* Disable MT7530 CKG_LNKDN_GLB */
	gswPbusWrite(0x30f0, 0x1e02);

	/* Pass Jumbo Frames up to 12K */
	gswPbusWrite(0x30e0, 0x3f33);

	/* Clear VLAN Table */
	mt7530_clear_all_vlan_table();

	/* Check switch ID = 7530 */
	regValue = gswPbusRead(0x7ffc);
	if((regValue & 0xffff0000) == 0x75300000)
		Serial.println(F("SUCCESS!"));
	else
		Serial.println(F("ERROR!"));
	/* Init priority table */
	mt7530_prio_init();
}

void getString()
{
	while (1)
	{
		while (Serial.available() <= 0) {}
		char c = Serial.read();
		if(((c < 0x30) && (c != 0xd)) || (c > 0x39))
			continue;
		Serial.print(c);
		if(c == 0xd) break;
		readString += c;
		delay(3);
	}
}

int16_t input_vlan_id()
{
	int32_t res = 0;

	while(1) {
		Serial.print(F("Set VLAN ID (1~4094): "));
		getString();
		res = atoi(&readString[0]);
		readString = "";
		if((res < 1) || (res > 4094)) {
			Serial.println(F("\nNot valid VLAN ID"));
			delay(300);
			continue;
		} else
			break;
	}
	Serial.println(F(""));
	return (int16_t)res & 0xfff;
}

int8_t get_yes_no()
{
	while (1)
	{
		while (Serial.available() <= 0) {}
		char c = Serial.read();
		if((c == 'y') || (c == 'Y')) {
			Serial.print(c);
			return 1;
		}
		if((c == 'n') || (c == 'N')) {
			Serial.print(c);
			return 0;
		}
		delay(3);
	}
	return -1;
}

int8_t input_ports_mask()
{
	int8_t b = 0, mask = 0;

	for (int i = 0; i < MAX_PORTS; i++) {
		Serial.print(F("Port"));
		Serial.print(i + 1, DEC);
		Serial.print(F(": "));
		b = get_yes_no();
		mask |= b << PORT_INV(i);
		Serial.println(F(""));
		delay(3);
	}
	return mask;
}

int8_t input_prio_set()
{
	int8_t b = 0, prio = 0;
	b = get_yes_no();
	Serial.println(F(""));
	if(!b)
		return 0;
	Serial.print(F("Num priority (1~7): "));
	while (1) {
		while (Serial.available() <= 0) {}
		int c = Serial.read() - '0';
		if (c > 0 && c < 8) {
			Serial.println(c);
			return (int8_t)c;
		}
	}
	return -1;
}

void eeprom_erase()
{
#if !defined(__AVR__)
	EEPROM.format();
#endif
	for (int i = 0; i < CFG_SIZE /* EEPROM.length() */; i++) { /* 5 + 32(4 * 8 VID) = 37 */
		EEPROM.write(i, 0);
	}

}

void reset_factory_defaults()
{
	Serial.print(F("\nErase Configuration..."));
	eeprom_erase();
	Serial.println(F("OK"));
	mt7530_init();
}

int check_magic(int write)
{
	int16_t magic = 0;

	EEPROM_get(0, magic);

	if(magic == MAGIC_EEPROM_START)
		return 1;
	if(write) {
		EEPROM_put(0, MAGIC_EEPROM_START);
		return 1;
	}
	if(magic == 0xffff) /* Force convert 0xff to 0x00 */
		eeprom_erase();
	return 0;
}

int check_idx(int write)
{
	int idx = EEPROM.read(2);
	if(write >= 0) {
		idx |= 1 << write;
		EEPROM.update(2, idx);
	}
	return idx;
}

void del_idx(int num)
{
	int addr = 5, idx = EEPROM.read(2);
	eeprom_vlan_record vlan;

	idx &= ~(1 << num);
	EEPROM.update(2, idx);
	vlan.vid = 0;
	vlan.ports_mask = 0;
	addr += sizeof(eeprom_vlan_record) * num;
	EEPROM_put(addr, vlan);
}

void load_configuration()
{
	eeprom_vlan_record vlan;
	int i, j, idx, addr = 5;
	int8_t ports_mask = 0, pvid_mask = 0;

	if(!check_magic(0))
		return;

	if((idx = check_idx(-1)) == 0)
		return;

	/* UnTagget ports */
	ports_mask = EEPROM.read(3);
	for (i = 0; i < MAX_PORTS; i++) {
		if((ports_mask & (1 << i)) != 0)
			mt7530_port_untagget(i);
	}

	/* Tagget ports */
	ports_mask = EEPROM.read(4);
	for (i = 0; i < MAX_PORTS; i++) {
		if((ports_mask & (1 << i)) != 0)
			mt7530_port_tagget(i);
	}

	for (i = 0; i < MAX_VLAN_GROUP; i++) {
		if((idx & (1 << i)) == 0) {
			addr += sizeof(eeprom_vlan_record);
			continue;
		}
		EEPROM_get(addr, vlan);
		/* Set PVID */
		pvid_mask = vlan.ports_mask & ~(ports_mask);
		for (j = 0; j < MAX_PORTS; j++) {
			if((pvid_mask & (1 << j)) != 0)
				mt7530_port_set_vid(j, vlan.vid);
		}
		/* Set VLAN */
		mt7530_add_vlan_table(vlan.ports_mask, vlan.vid, vlan.prio);

		addr += sizeof(eeprom_vlan_record);
	}
}

void show_configuration()
{
	eeprom_vlan_record vlan;
	int i, j, idx, addr = 5;
	int8_t ports_mask = 0;

	Serial.println(F(""));
	if(!check_magic(0)) {
		Serial.println(F("Configuration is empty!"));
		return;
	}

	if((idx = check_idx(-1)) == 0) {
		Serial.println(F("VLAN groups is empty!"));
		return;
	}

	Serial.println(F("---------- Configuration ----------"));
	Serial.println(F("       [ Arduino Firmware ]"));
	Serial.print(F("Version: "));
	Serial.println(F(VERSION));
	Serial.println(F("       [ Ports Groups ]"));
	/* UnTagged ports */
	Serial.print(F("UnTagged ports: "));
	ports_mask = EEPROM.read(3);
	for (i = 0; i < MAX_PORTS; i++) {
		if((ports_mask & (1 << i)) != 0) {
			Serial.print(PORT_INV(i) + 1, DEC);
			Serial.print(F(" "));
		}
	}
	Serial.println(F(""));

	/* Tagged ports */
	Serial.print(F("Tagged ports:   "));
	ports_mask = EEPROM.read(4);
	for (i = 0; i < MAX_PORTS; i++) {
		if((ports_mask & (1 << i)) != 0) {
			Serial.print(PORT_INV(i) + 1, DEC);
			Serial.print(F(" "));
		}
	}
	Serial.println(F(""));

	/* VLAN Groups */
	Serial.println(F("       [ VLAN Groups ]"));
	for (i = 0; i < MAX_VLAN_GROUP; i++) {
		if((idx & (1 << i)) == 0) {
			addr += sizeof(eeprom_vlan_record);
			continue;
		}
		EEPROM_get(addr, vlan);
		Serial.print(F("IDX"));
		Serial.print(i + 1, DEC);
		Serial.print(F(": Ports: "));
		for (j = 0; j < MAX_PORTS; j++) {
			if((vlan.ports_mask & (1 << j)) != 0) {
				Serial.print(PORT_INV(j) + 1, DEC);
				Serial.print(F(" "));
			} else
				Serial.print(F("- "));
		}
		Serial.print(F("VID: "));
		char fvid[4];
		sprintf(fvid, "%04d", vlan.vid);
		Serial.print(fvid);
		Serial.print(F(" Priority: "));
		Serial.println(vlan.prio, DEC);

		addr += sizeof(eeprom_vlan_record);
	}
	Serial.println(F("-------- End Configuration --------"));
}

void setup()
{
	Serial.begin(115200);
#if !defined(__AVR__) /* flash 128Kbyte, if 64Kbyte use 0x800Fxxx */
	EEPROM.PageBase0 = 0x801F000;
	EEPROM.PageBase1 = 0x801F800;
	EEPROM.PageSize  = 0x400; /* set EEPROM Emulation size 1024 byte, if need 2048 = 0x800 */
	EEPROM.init();
	delay(2000);
#else
	delay(50);
#endif
	Serial.println(F("\n"));
	Serial.setTimeout(60L*60L*1000L);
	pinMode(PIN_MDIO, INPUT);
	pinMode(PIN_MDC, INPUT);
#if defined(__AVR__)
	pinMode(PIN_GND, INPUT); /* Not use */
#endif
	Serial.println(F("\nWaiting for the switch chip to start-up (3 sec)..."));
	mt7530_init();
#if 0
	mt7530_set_vlan_example();
#else
	load_configuration();
#endif
	delay(1000);
}

void loop()
{
	Serial.println(F("\nMT7530D switch setup"));
	Serial.println(F("0. Link Status"));
	Serial.println(F("1. Show VLAN Table from switch"));
	Serial.println(F("2. Show system configuration"));
	Serial.println(F("3. Add Tagged ports group"));
	Serial.println(F("4. Add UnTagged ports group"));
	Serial.println(F("5. Add VLAN ID, ports group and priority"));
	Serial.println(F("6. Delete VLAN ID, ports group and priority by IDX"));
	Serial.println(F("7. Apply new configuration"));
	Serial.println(F("8. Reset in factory defaults"));
	Serial.print(F("Enter you choice: "));
	int c, ch = -1, i, idx, addr;
	int8_t mask;
	while (1)
	{
		while (Serial.available() <= 0) {}
		c = Serial.read();
#if !defined(__AVR__)
		if(c == 0xd) { /* for STM32 board fix */
			Serial.println(F(""));
			break;
		}
#endif
		ch = c - '0';
		if (ch >= 0 && ch < 9) break;
		delay(3);
	}
	if (ch >= 0 && ch < 9)
	{
		Serial.println(ch);
		switch(ch) {
			case 0:
				mt7530_link_status();
				break;
			case 1:
				mt7530_vlan_table_dump();
				break;
			case 2:
				show_configuration();
				break;
			case 3:
				{
					Serial.println(F("\nTagged ports group set (y/n):"));
					mask = input_ports_mask();
					EEPROM.update(4, mask);
					check_magic(1);
					Serial.println(F("Save configuration OK!"));
					break;
				}
			case 4:
				{
					Serial.println(F("\nUnTagged ports group set (y/n):"));
					mask = input_ports_mask();
					EEPROM.update(3, mask);
					check_magic(1);
					Serial.println(F("Save configuration OK!"));
					break;
				}
			case 5:
				{
					int free_idx = 0;
					addr = 5;
					if((idx = check_idx(-1)) == 0xff) {
						Serial.println(F("\nTable VLAN ID and ports group is FULL!"));
						break;
					}
					if(!EEPROM.read(3) && !EEPROM.read(4)) {
						Serial.println(F("\nPlease first set masks Tagged or UnTagged ports group!"));
						break;
					}
					eeprom_vlan_record vlan;
					for (i = 0; i < MAX_VLAN_GROUP; i++) { /* Get free idx */
						if((idx & (1 << i)) == 0) {
							free_idx = i;
							addr += sizeof(eeprom_vlan_record) * i;
							break;
						}
					}
					vlan.vid = input_vlan_id();
					Serial.print(F("VLAN ID set: "));
					Serial.println(vlan.vid, DEC);
					Serial.println(F("Ports group set (y/n):"));
					vlan.ports_mask = input_ports_mask();
					Serial.print(F("Priority set (y/n): "));
					vlan.prio = input_prio_set();
					Serial.println(F(""));
					check_magic(1);
					check_idx(free_idx);
					EEPROM_put(addr, vlan);
					Serial.println(F("Save configuration OK!"));
					break;
				}
			case 6:
				{
					Serial.print(F("\nEnter number IDX for delete (1~8): "));
					while (1)
					{
						while (Serial.available() <= 0) {}
						idx = Serial.read()-'0';
						if (idx > 0 && idx < 9)
							break;
						delay(3);
					}
					Serial.println(idx);
					del_idx(idx - 1);
					Serial.print(F("Delete IDX"));
					Serial.print(idx, DEC);
					Serial.println(F(" and save configuration OK!"));
					break;
				}
			case 7:
				mt7530_init();
				load_configuration();
				break;
			case 8:
				reset_factory_defaults();
				break;
		}
		delay(1000);
	}
}
