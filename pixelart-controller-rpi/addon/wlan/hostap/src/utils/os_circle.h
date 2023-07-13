//
// os_circle.h
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
#ifndef _os_circle_h
#define _os_circle_h

#include <circle/alloc.h>
#include <circle/stdarg.h>
#include <circle/util.h>
#include <circle/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "os.h"

#define __LITTLE_ENDIAN	1234
#define __BYTE_ORDER	__LITTLE_ENDIAN
#define WPA_TYPES_DEFINED

#define NULL		0

#define __force

#define bswap_16	bswap16
#define bswap_32	bswap32

typedef signed long	time_t;

#define abs		__wpa_abs
#define isprint		__wpa_isprint
#define strrchr		__wpa_strrchr
#define strdup		__wpa_strdup
#define snprintf	__wpa_snprintf
#define vsnprintf	__wpa_vsnprintf
#define printf		__wpa_printf
#define vprintf		__wpa_vprintf
#define qsort		__wpa_qsort
#define abort		__wpa_abort

int abs (int i);

int isprint (int c);

char *strrchr (const char *s, int c);
char *strdup (const char *s);

int snprintf (char *str, size_t size, const char *format, ...);
int vsnprintf (char *str, size_t size, const char *format, va_list ap);
int printf (const char *format, ...);
int vprintf (const char *format, va_list ap);

void qsort (void *base, size_t nmemb, size_t size,
	    int (*compare) (const void *, const void *));

void abort (void);

void os_hexdump (const char *title, const void *p, size_t len);

#define os_daemonize(s)			(0)
#define os_daemonize_terminate(s)	((void) 0)

#ifdef __cplusplus
}
#endif

#endif
