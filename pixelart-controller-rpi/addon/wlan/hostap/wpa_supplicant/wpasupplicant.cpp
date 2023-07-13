//
// wpasupplicant.cpp
//
// WPA Supplicant wrapper class for Circle
// by R. Stange <rsta2@o2online.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Alternatively, this software may be distributed under the terms of BSD
// license.
//
#include "wpasupplicant.h"
#include <wlan/p9compat.h>
#include <assert.h>

extern "C"
{

#include "eloop.h"

}

int wpa_supplicant_main (const char *confname);
int wpa_supplicant_is_connected (void);

CWPASupplicant::CWPASupplicant (const char *pConfigFile)
:	m_ConfigFile (pConfigFile)
{
	m_ShutdownEvent.Clear ();
}

CWPASupplicant::~CWPASupplicant (void)
{
	eloop_terminate ();

	m_ShutdownEvent.Wait ();
}

boolean CWPASupplicant::Initialize (void)
{
	kproc ("wpa_supplicant", ProcEntry, this);

	return TRUE;
}

boolean CWPASupplicant::IsConnected (void) const
{
	return !!wpa_supplicant_is_connected ();
}

void CWPASupplicant::ProcEntry (void *pParam)
{
	CWPASupplicant *pThis = (CWPASupplicant *) pParam;
	assert (pThis != 0);

	wpa_supplicant_main (pThis->m_ConfigFile);

	pThis->m_ShutdownEvent.Set ();
}
