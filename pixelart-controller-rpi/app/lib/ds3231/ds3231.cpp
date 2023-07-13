//
// ds3231.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2016  R. Stange <rsta2@o2online.de>
//
// Portions by Arjan van Vught <info@raspberrypi-dmx.nl>
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
#include <ds3231/ds3231.h>
#include <circle/logger.h>
#include <circle/util.h>
#include <assert.h>

const char FromDS3231[] = "rtc";

CDS3231::CDS3231 (CI2CMaster *pI2CMaster, unsigned nI2CClockHz, u8 ucSlaveAddress)
:	m_pI2CMaster (pI2CMaster),
	m_nI2CClockHz (nI2CClockHz),
	m_ucSlaveAddress (ucSlaveAddress)
{
}

CDS3231::~CDS3231 (void)
{
	m_pI2CMaster = 0;
}

boolean CDS3231::Initialize (void)
{
	assert (m_pI2CMaster != 0);
	m_pI2CMaster->SetClock (m_nI2CClockHz);

	if (m_pI2CMaster->Write (m_ucSlaveAddress, 0, 0) != 0)
	{
		CLogger::Get ()->Write (FromDS3231, LogError, "Device is not present (addr 0x%02X)",
					(unsigned) m_ucSlaveAddress);

		return FALSE;
	}

	return TRUE;
}

boolean CDS3231::Get (CTime *pTime)
{
	assert (m_pI2CMaster != 0);
	m_pI2CMaster->SetClock (m_nI2CClockHz);

	u8 Cmd[] = {DS3231_TIME_CAL_ADDR};
	int nResult = m_pI2CMaster->Write (m_ucSlaveAddress, Cmd, sizeof Cmd);
	if (nResult != sizeof Cmd)
	{
		CLogger::Get ()->Write (FromDS3231, LogError, "I2C write failed (err %d)", nResult);

		return FALSE;
	}

	u8 Reg[7];
	nResult = m_pI2CMaster->Read (m_ucSlaveAddress, Reg, sizeof Reg);
	if (nResult != sizeof Reg)
	{
		CLogger::Get ()->Write (FromDS3231, LogError, "I2C read failed (err %d)", nResult);

		return FALSE;
	}

	assert (pTime != 0);
	if(Reg[DS3231_RTCC_TCR_HOURS] & DS3231_RTCC_BIT_12)
	{
		// 12 hour mode
		if (!pTime->SetTime (BCD2DEC (Reg[DS3231_RTCC_TCR_HOURS] & 0x1F) + ((Reg[DS3231_RTCC_TCR_HOURS] & 0x20) >> 5) * 12,
					BCD2DEC (Reg[DS3231_RTCC_TCR_MINUTES] & 0x7F),
					BCD2DEC (Reg[DS3231_RTCC_TCR_SECONDS] & 0x7F)))
		{
			CLogger::Get ()->Write (FromDS3231, LogError, "Invalid time read");

			return FALSE;
		}
	}
	else
	{
		//24 hour mode
		if (!pTime->SetTime (BCD2DEC (Reg[DS3231_RTCC_TCR_HOURS] & 0x3F),
					BCD2DEC (Reg[DS3231_RTCC_TCR_MINUTES] & 0x7F),
					BCD2DEC (Reg[DS3231_RTCC_TCR_SECONDS] & 0x7F)))
		{
			CLogger::Get ()->Write (FromDS3231, LogError, "Invalid time read");

			return FALSE;
		}
	}

	if (!pTime->SetDate (BCD2DEC (Reg[DS3231_RTCC_TCR_DATE]  & 0x3F),
			     BCD2DEC (Reg[DS3231_RTCC_TCR_MONTH] & 0x1F),
			     BCD2DEC (Reg[DS3231_RTCC_TCR_YEAR]) + 2000 + (((Reg[DS3231_RTCC_TCR_MONTH] & 0x80) >> 7) * 100)))
	{
		CLogger::Get ()->Write (FromDS3231, LogError, "Invalid date read");

		return FALSE;
	}

	return TRUE;
}

boolean CDS3231::Set (const CTime &Time)
{
	u8 Reg[7];
	Reg[DS3231_RTCC_TCR_SECONDS] = DEC2BCD ((u8) Time.GetSeconds()     & 0x7F);
	Reg[DS3231_RTCC_TCR_MINUTES] = DEC2BCD ((u8) Time.GetMinutes()     & 0x7F);
	Reg[DS3231_RTCC_TCR_HOURS]   = DEC2BCD ((u8) Time.GetHours()       & 0x1F);
	Reg[DS3231_RTCC_TCR_DAY]     = DEC2BCD ((u8) (Time.GetWeekDay() + 1) & 0x07);
	Reg[DS3231_RTCC_TCR_DATE]    = DEC2BCD ((u8) Time.GetMonthDay()    & 0x3F);
	Reg[DS3231_RTCC_TCR_MONTH]   = DEC2BCD ((u8) (Time.GetMonth()       & 0x1F) + (0x80 * (Time.GetYear() >= 2100)));
	Reg[DS3231_RTCC_TCR_YEAR]    = DEC2BCD ((u8) (Time.GetYear() % 1000) & 0xFF);

	u8 Data[8] = {DS3231_TIME_CAL_ADDR};
	memcpy (Data+1, Reg, sizeof Reg);

	assert (m_pI2CMaster != 0);
	m_pI2CMaster->SetClock (m_nI2CClockHz);

	unsigned nResult = m_pI2CMaster->Write (m_ucSlaveAddress, Data, sizeof Data);
	if (nResult != sizeof Data)
	{
		CLogger::Get ()->Write (FromDS3231, LogError, "I2C write failed (err %d)", nResult);

		return FALSE;
	}

	return TRUE;
}
