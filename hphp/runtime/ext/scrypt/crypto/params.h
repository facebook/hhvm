#ifndef SCRYPT_PARAMS_H
#define SCRYPT_PARAMS_H
/*-
 * Copyright 2009 Colin Percival
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file was originally written by Colin Percival as part of the Tarsnap
 * online backup system.
 */

#include <stddef.h>

/**
 * The parameters maxmem, maxmemfrac, and maxtime used by all of these
 * functions are defined as follows:
 * maxmem - maximum number of bytes of storage to use for V array (which is
 *     by far the largest consumer of memory).  If this value is set to 0, no
 *     maximum will be enforced; any other value less than 1 MiB will be
 *     treated as 1 MiB.
 * maxmemfrac - maximum fraction of available storage to use for the V array,
 *     where "available storage" is defined as the minimum out of the
 *     RLIMIT_AS, RLIMIT_DATA. and RLIMIT_RSS resource limits (if any are
 *     set).  If this value is set to 0 or more than 0.5 it will be treated
 *     as 0.5; and this value will never cause a limit of less than 1 MiB to
 *     be enforced.
 * maxtime - maximum amount of CPU time to spend computing the derived keys,
 *     in seconds.  This limit is only approximately enforced; the CPU
 *     performance is estimated and parameter limits are chosen accordingly.
 * For the encryption functions, the parameters to the scrypt key derivation
 * function are chosen to make the key as strong as possible subject to the
 * specified limits; for the decryption functions, the parameters used are
 * compared to the computed limits and an error is returned if decrypting
 * the data would take too much memory or CPU time.
 */

int pickparams(size_t maxmem, double maxmemfrac, double maxtime, int * logN,
               uint32_t * r, uint32_t * p);
int checkparams(size_t maxmem, double maxmemfrac, double maxtime, int logN,
                uint32_t r, uint32_t p);

#endif /* !SCRYPT_PARAMS_H */
