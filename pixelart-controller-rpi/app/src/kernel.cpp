//
// kernel.cpp
//
#include "kernel.h"

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_I2CMaster(1),
	m_RTC (&m_I2CMaster)
{
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
		bOK = m_Serial.Initialize(115200);

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice(m_Options.GetLogDevice (), FALSE);
		if (pTarget != 0)
			bOK = m_Logger.Initialize(pTarget);
	}

	if (bOK)
		bOK = m_Interrupt.Initialize();

	if (bOK)
		bOK = m_Timer.Initialize();

	if(bOK)
		bOK = m_I2CMaster.Initialize();

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write(FromKernel, LogDebug, "Starting kernel");



	return ShutdownHalt;
}
