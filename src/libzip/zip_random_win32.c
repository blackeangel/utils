/*
  zip_random_win32.c -- fill the user's buffer with random stuff (Windows version)
  Copyright (C) 2016-2024 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <info@libzip.org>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "zipint.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 /* Windows 7+: required for bcrypt.h */
#endif

#include <windows.h>
#include <bcrypt.h>
#include <stdlib.h>
#include <time.h>

#ifndef HAVE_SECURE_RANDOM
ZIP_EXTERN bool zip_secure_random(zip_uint8_t *buffer, zip_uint16_t length) {
    NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)buffer, (ULONG)length,
                                       BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return BCRYPT_SUCCESS(status);
}
#endif

#ifndef HAVE_RANDOM_UINT32
zip_uint32_t zip_random_uint32(void) {
    static bool seeded = false;
    zip_uint32_t value;

    if (zip_secure_random((zip_uint8_t *)&value, sizeof(value))) {
        return value;
    }

    /* Fallback if BCryptGenRandom fails (should not happen on Vista+) */
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
    return (zip_uint32_t)rand() ^ ((zip_uint32_t)rand() << 16);
}
#endif
