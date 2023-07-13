//
// ds3231.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2016  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _ds3231_ds3231_h
#define _ds3231_ds3231_h

#include <circle/i2cmaster.h>
#include <circle/time.h>
#include <circle/types.h>

// i2c address of the ds3231 chip
#define DS3231_I2C_ADDR             0x68

// timekeeping registers
#define DS3231_TIME_CAL_ADDR        0x00
#define DS3231_ALARM1_ADDR          0x07
#define DS3231_ALARM2_ADDR          0x0B
#define DS3231_CONTROL_ADDR         0x0E
#define DS3231_STATUS_ADDR          0x0F
#define DS3231_AGING_OFFSET_ADDR    0x10
#define DS3231_TEMPERATURE_ADDR     0x11

// Time Registers (TCR)
#define DS3231_RTCC_TCR_SECONDS		0x00
#define DS3231_RTCC_TCR_MINUTES		0x01
#define DS3231_RTCC_TCR_HOURS		0x02
#define DS3231_RTCC_BIT_12			0x40
#define DS3231_RTCC_TCR_DAY			0x03
#define DS3231_RTCC_TCR_DATE		0x04
#define DS3231_RTCC_TCR_MONTH		0x05
#define DS3231_RTCC_TCR_YEAR		0x06

// control register bits
#define DS3231_CONTROL_A1IE     0x1		/* Alarm 2 Interrupt Enable */
#define DS3231_CONTROL_A2IE     0x2		/* Alarm 2 Interrupt Enable */
#define DS3231_CONTROL_INTCN    0x4		/* Interrupt Control */
#define DS3231_CONTROL_RS1	    0x8		/* square-wave rate select 2 */
#define DS3231_CONTROL_RS2    	0x10	/* square-wave rate select 2 */
#define DS3231_CONTROL_CONV    	0x20	/* Convert Temperature */
#define DS3231_CONTROL_BBSQW    0x40	/* Battery-Backed Square-Wave Enable */
#define DS3231_CONTROL_EOSC	    0x80	/* not Enable Oscillator, 0 equal on */

// status register bits
#define DS3231_STATUS_A1F      0x01		/* Alarm 1 Flag */
#define DS3231_STATUS_A2F      0x02		/* Alarm 2 Flag */
#define DS3231_STATUS_BUSY     0x04		/* device is busy executing TCXO */
#define DS3231_STATUS_EN32KHZ  0x08		/* Enable 32KHz Output  */
#define DS3231_STATUS_OSF      0x80		/* Oscillator Stop Flag */

#define SECONDS_FROM_1970_TO_2000 946684800

#define BCD2DEC(val)			(((val) & 0x0F) + ((val) >> 4) * 10)
#define DEC2BCD(val)			((((val) / 10) << 4) + (val) % 10)

class CDS3231
{
public:
	CDS3231 (CI2CMaster *pI2CMaster, unsigned nI2CClockHz = 100000, u8 ucSlaveAddress = DS3231_I2C_ADDR);
	~CDS3231 (void);

	boolean Initialize (void);

	boolean Get (CTime *pTime);
	boolean Set (const CTime &Time);

private:
	CI2CMaster *m_pI2CMaster;
	unsigned    m_nI2CClockHz;
	u8	    m_ucSlaveAddress;
};

#endif
