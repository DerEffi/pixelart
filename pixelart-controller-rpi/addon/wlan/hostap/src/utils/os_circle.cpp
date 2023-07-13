//
// os_circle.cpp
//
// OS specific functions for Circle
// by R. Stange <rsta2@o2online.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Alternatively, this software may be distributed under the terms of BSD
// license.
//
#include "os_circle.h"
#include <circle/string.h>
#include <circle/logger.h>
#include <circle/timer.h>
#include <circle/startup.h>
#include <circle/bcmrandom.h>
#include <circle/sysconfig.h>
#include <circle/multicore.h>
#include <circle/debug.h>
#include <circle/sched/scheduler.h>
#include <assert.h>

static const char From[] = "wpa";

int abs (int i)
{
	return i < 0 ? -i : i;
}

int isprint (int c)
{
	return ' ' <= c && c <= 126;
}

char *strrchr (const char *s, int c)
{
	while (*s)
	{
		s++;
	}

	while (*--s)
	{
		if (*s == c)
		{
			return (char *) s;
		}
	}

	return 0;
}

char *strdup (const char *s)
{
	size_t size = strlen (s) + 1;

	char *p = (char *) malloc (size);
	assert (p != 0);

	return strcpy (p, s);
}

int snprintf (char *str, size_t size, const char *format, ...)
{
	va_list ap;
	va_start (ap, format);

	int res = vsnprintf (str, size, format, ap);

	va_end (ap);

	return res;
}

int vsnprintf (char *str, size_t size, const char *format, va_list ap)
{
	CString Msg;
	Msg.FormatV (format, ap);

	strncpy (str, (const char *) Msg, size);
	str[size-1] = '\0';

	return Msg.GetLength () < size-1 ? Msg.GetLength () : size-1;
}

int printf (const char *format, ...)
{
	va_list ap;
	va_start (ap, format);

	int res = vprintf (format, ap);

	va_end (ap);

	return res;
}

int vprintf (const char *format, va_list ap)
{
	CString Msg;
	Msg.FormatV (format, ap);

	char Buffer[1000];
	strncpy (Buffer, (const char *) Msg, sizeof Buffer);
	Buffer[sizeof Buffer-1] = '\0';

	char *pLine;
	char *pLineEnd;
	for (pLine = Buffer; (pLineEnd = strchr (pLine, '\n')) != 0; pLine = pLineEnd+1)
	{
		*pLineEnd = '\0';

		if (*pLine != '\0')
		{
			CLogger::Get ()->Write (From, LogDebug, "%s", pLine);
		}
	}

	if (*pLine != '\0')
	{
		CLogger::Get ()->Write (From, LogDebug, "%s", pLine);
	}

	return Msg.GetLength ();
}

// TODO: this is not quick sort
void qsort (void *base, size_t nmemb, size_t size, int (*compare) (const void *, const void *))
{
	if (nmemb <= 1)
	{
		return;
	}

	assert (base != 0);
	u8 *array = (u8 *) base;

	assert (size > 0);
	u8 tmp[size];

	assert (compare != 0);
	for (unsigned i = 0; i < nmemb-1; i++)
	{
		for (unsigned j = i+1; j < nmemb; j++)
		{
#define ELEM(n)	&array[(n)*size]
			if ((*compare) (ELEM (i), ELEM (j)) > 0)
			{
				memcpy (tmp, ELEM (i), size);
				memcpy (ELEM (i), ELEM (j), size);
				memcpy (ELEM (j), tmp, size);
			}
		}
	}
}

void abort (void)
{
#ifndef ARM_ALLOW_MULTI_CORE
	halt ();
#else
	CMultiCoreSupport::HaltAll ();
#endif
}

int os_get_random (unsigned char *buf, size_t len)
{
	static CBcmRandomNumberGenerator *s_pRNG = 0;

	if (s_pRNG == 0)
	{
		s_pRNG = new CBcmRandomNumberGenerator ();
		assert (s_pRNG != 0);
	}

	while (len >= sizeof (u32))
	{
		*(u32 *) buf = s_pRNG->GetNumber ();

		buf += sizeof (u32);
		len -= sizeof (u32);
	}

	if (len > 0)
	{
		u32 num = s_pRNG->GetNumber ();

		switch (len)
		{
		case 3:
			buf[2] = (num >> 16) & 0xFF;
			// fall through

		case 2:
			buf[1] = (num >> 8) & 0xFF;
			// fall through

		case 1:
			buf[0] = num & 0xFF;
			break;

		default:
			assert (0);
			break;
		}
	}

	return 0;
}

int os_get_time (struct os_time *t)
{
	unsigned sec, usec;
	CTimer::Get ()->GetLocalTime (&sec, &usec);

	t->sec = sec;
	t->usec = usec;

	return 0;
}

void os_sleep (os_time_t sec, os_time_t usec)
{
	if (sec)
	{
		CScheduler::Get ()->Sleep (sec);
	}

	if (usec)
	{
		CScheduler::Get ()->usSleep (usec);
	}
}

size_t os_strlcpy (char *dest, const char *src, size_t siz)
{
	assert (siz > 0);

	strncpy (dest, src, siz);
	dest[siz-1] = '\0';

	return strlen (dest);
}

void * os_zalloc (size_t size)
{
	void *p = malloc (size);
	assert (p != 0);

	memset (p, 0, size);

	return p;
}

void os_hexdump (const char *title, const void *p, size_t len)
{
#ifndef NDEBUG
	debug_hexdump (p, len, title);
#endif
}
