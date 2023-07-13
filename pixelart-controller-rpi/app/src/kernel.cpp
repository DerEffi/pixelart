//
// kernel.cpp
//
#include "kernel.h"

#define DRIVE				"SD:"
#define FIRMWARE_PATH		DRIVE "/firmware/"		// firmware files must be provided here
#define CONFIG_FILE		DRIVE "/wifi.conf"

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_I2CMaster(1),
	m_RTC (&m_I2CMaster),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
	m_WLAN (FIRMWARE_PATH),
	m_Net (0, 0, 0, 0, DEFAULT_HOSTNAME, NetDeviceTypeWLAN),
	m_WPASupplicant (CONFIG_FILE)
{
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if(bOK)
		bOK = m_Serial.Initialize(115200);

	if(bOK) {
		CDevice *pTarget = m_DeviceNameService.GetDevice(m_Options.GetLogDevice (), FALSE);
		if (pTarget != 0)
			bOK = m_Logger.Initialize(pTarget);
	}

	if(bOK)
		bOK = m_Interrupt.Initialize();

	if(bOK)
		bOK = m_Timer.Initialize();

	if(bOK)
		bOK = m_I2CMaster.Initialize();

	if(bOK)
		bOK = m_EMMC.Initialize();

	if(bOK && f_mount(&m_FileSystem, DRIVE, 1) != FR_OK) {
		m_Logger.Write(FromKernel, LogError, "Cannot mount drive: %s", DRIVE);
		bOK = FALSE;
	}

	if(bOK)
		bOK = m_WLAN.Initialize();

	if(bOK)
		bOK = m_Net.Initialize(FALSE);

	if(bOK)
		bOK = m_WPASupplicant.Initialize();

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write(FromKernel, LogDebug, "Starting kernel");

	while(!m_Net.IsRunning())
		m_Scheduler.MsSleep(100);

	CString IPString;
	m_Net.GetConfig()->GetIPAddress()->Format(&IPString);
	m_Logger.Write(FromKernel, LogNotice, "Try \"ping %s\" from another computer!", (const char *) IPString);

	return ShutdownHalt;
}
