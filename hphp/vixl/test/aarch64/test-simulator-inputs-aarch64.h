// Copyright 2015, VIXL authors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of ARM Limited nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This file holds inputs for the instructions tested by test-simulator-aarch64.
//
// If the input lists are updated, please run tools/generate_simulator_traces.py
// on a reference platform to regenerate the expected outputs. The outputs are
// stored in test-simulator-traces-aarch64.h.

extern "C" {
#include <stdint.h>
}

// This header should only be used by test/test-simulator-aarch64.cc, so it
// doesn't need the usual header guard.
#ifdef VIXL_AARCH64_TEST_SIMULATOR_INPUTS_AARCH64_H_
#error This header should be included only once.
#endif
#define VIXL_AARCH64_TEST_SIMULATOR_INPUTS_AARCH64_H_

// clang-format off

// Double values, stored as uint64_t representations. This ensures exact bit
// representation, and avoids the loss of NaNs and suchlike through C++ casts.
#define INPUT_DOUBLE_BASIC                                                    \
  /* Simple values. */                                                        \
  0x0000000000000000,   /* 0.0                        */                      \
  0x0010000000000000,   /* The smallest normal value. */                      \
  0x3fdfffffffffffff,   /* The value just below 0.5.  */                      \
  0x3fe0000000000000,   /* 0.5                        */                      \
  0x3fe0000000000001,   /* The value just above 0.5.  */                      \
  0x3fefffffffffffff,   /* The value just below 1.0.  */                      \
  0x3ff0000000000000,   /* 1.0                        */                      \
  0x3ff0000000000001,   /* The value just above 1.0.  */                      \
  0x3ff8000000000000,   /* 1.5                        */                      \
  0x4024000000000000,   /* 10                         */                      \
  0x7fefffffffffffff,   /* The largest finite value.  */                      \
                                                                              \
  /* Infinity. */                                                             \
  0x7ff0000000000000,                                                         \
                                                                              \
  /* NaNs. */                                                                 \
  /*  - Quiet NaNs */                                                         \
  0x7ff923456789abcd,                                                         \
  0x7ff8000000000000,                                                         \
  /*  - Signalling NaNs */                                                    \
  0x7ff123456789abcd,                                                         \
  0x7ff0000000000000,                                                         \
                                                                              \
  /* Subnormals. */                                                           \
  /*  - A recognisable bit pattern. */                                        \
  0x000123456789abcd,                                                         \
  /*  - The largest subnormal value. */                                       \
  0x000fffffffffffff,                                                         \
  /*  - The smallest subnormal value. */                                      \
  0x0000000000000001,                                                         \
                                                                              \
  /* The same values again, but negated. */                                   \
  0x8000000000000000,                                                         \
  0x8010000000000000,                                                         \
  0xbfdfffffffffffff,                                                         \
  0xbfe0000000000000,                                                         \
  0xbfe0000000000001,                                                         \
  0xbfefffffffffffff,                                                         \
  0xbff0000000000000,                                                         \
  0xbff0000000000001,                                                         \
  0xbff8000000000000,                                                         \
  0xc024000000000000,                                                         \
  0xffefffffffffffff,                                                         \
  0xfff0000000000000,                                                         \
  0xfff923456789abcd,                                                         \
  0xfff8000000000000,                                                         \
  0xfff123456789abcd,                                                         \
  0xfff0000000000000,                                                         \
  0x800123456789abcd,                                                         \
  0x800fffffffffffff,                                                         \
  0x8000000000000001,


// Extra inputs. Passing these to 3- or 2-op instructions makes the trace file
// very large, so these should only be used with 1-op instructions.
#define INPUT_DOUBLE_CONVERSIONS                                              \
  /* Values relevant for conversions to single-precision floats. */           \
  0x47efffff00000000,                                                         \
  /*  - The smallest normalized float. */                                     \
  0x3810000000000000,                                                         \
  /*  - Normal floats that need (ties-to-even) rounding.           */         \
  /*    For normalized numbers, bit 29 (0x0000000020000000) is the */         \
  /*    lowest-order bit which will fit in the float's mantissa.   */         \
  0x3ff0000000000000,                                                         \
  0x3ff0000000000001,                                                         \
  0x3ff0000010000000,                                                         \
  0x3ff0000010000001,                                                         \
  0x3ff0000020000000,                                                         \
  0x3ff0000020000001,                                                         \
  0x3ff0000030000000,                                                         \
  0x3ff0000030000001,                                                         \
  0x3ff0000040000000,                                                         \
  0x3ff0000040000001,                                                         \
  0x3ff0000050000000,                                                         \
  0x3ff0000050000001,                                                         \
  0x3ff0000060000000,                                                         \
  /*  - A mantissa that overflows into the exponent during rounding. */       \
  0x3feffffff0000000,                                                         \
  /*  - The largest double that rounds to a normal float. */                  \
  0x47efffffefffffff,                                                         \
  /*  - The smallest exponent that's too big for a float. */                  \
  0x47f0000000000000,                                                         \
  /*  - This exponent is in range, but the value rounds to infinity. */       \
  0x47effffff0000000,                                                         \
  /*  - The largest double which is too small for a subnormal float. */       \
  0x3690000000000000,                                                         \
  /*  - The largest subnormal float. */                                       \
  0x380fffffc0000000,                                                         \
  /*  - The smallest subnormal float. */                                      \
  0x36a0000000000000,                                                         \
  /*  - Subnormal floats that need (ties-to-even) rounding.      */           \
  /*    For these subnormals, bit 34 (0x0000000400000000) is the */           \
  /*    lowest-order bit which will fit in the float's mantissa. */           \
  0x37c159e000000000,                                                         \
  0x37c159e000000001,                                                         \
  0x37c159e200000000,                                                         \
  0x37c159e200000001,                                                         \
  0x37c159e400000000,                                                         \
  0x37c159e400000001,                                                         \
  0x37c159e600000000,                                                         \
  0x37c159e600000001,                                                         \
  0x37c159e800000000,                                                         \
  0x37c159e800000001,                                                         \
  0x37c159ea00000000,                                                         \
  0x37c159ea00000001,                                                         \
  0x37c159ec00000000,                                                         \
  /*  - The smallest double which rounds up to become a subnormal float. */   \
  0x3690000000000001,                                                         \
                                                                              \
  /* The same values again, but negated. */                                   \
  0xc7efffff00000000,                                                         \
  0xb810000000000000,                                                         \
  0xbff0000000000000,                                                         \
  0xbff0000000000001,                                                         \
  0xbff0000010000000,                                                         \
  0xbff0000010000001,                                                         \
  0xbff0000020000000,                                                         \
  0xbff0000020000001,                                                         \
  0xbff0000030000000,                                                         \
  0xbff0000030000001,                                                         \
  0xbff0000040000000,                                                         \
  0xbff0000040000001,                                                         \
  0xbff0000050000000,                                                         \
  0xbff0000050000001,                                                         \
  0xbff0000060000000,                                                         \
  0xbfeffffff0000000,                                                         \
  0xc7efffffefffffff,                                                         \
  0xc7f0000000000000,                                                         \
  0xc7effffff0000000,                                                         \
  0xb690000000000000,                                                         \
  0xb80fffffc0000000,                                                         \
  0xb6a0000000000000,                                                         \
  0xb7c159e000000000,                                                         \
  0xb7c159e000000001,                                                         \
  0xb7c159e200000000,                                                         \
  0xb7c159e200000001,                                                         \
  0xb7c159e400000000,                                                         \
  0xb7c159e400000001,                                                         \
  0xb7c159e600000000,                                                         \
  0xb7c159e600000001,                                                         \
  0xb7c159e800000000,                                                         \
  0xb7c159e800000001,                                                         \
  0xb7c159ea00000000,                                                         \
  0xb7c159ea00000001,                                                         \
  0xb7c159ec00000000,                                                         \
  0xb690000000000001,                                                         \
                                                                              \
  /* Values relevant for conversions to integers (frint).           */        \
  /*  - The lowest-order mantissa bit has value 1.                  */        \
  0x4330000000000000,                                                         \
  0x4330000000000001,                                                         \
  0x4330000000000002,                                                         \
  0x4330000000000003,                                                         \
  0x433fedcba9876543,                                                         \
  0x433ffffffffffffc,                                                         \
  0x433ffffffffffffd,                                                         \
  0x433ffffffffffffe,                                                         \
  0x433fffffffffffff,                                                         \
  /*  - The lowest-order mantissa bit has value 0.5.                */        \
  0x4320000000000000,                                                         \
  0x4320000000000001,                                                         \
  0x4320000000000002,                                                         \
  0x4320000000000003,                                                         \
  0x432fedcba9876543,                                                         \
  0x432ffffffffffffc,                                                         \
  0x432ffffffffffffd,                                                         \
  0x432ffffffffffffe,                                                         \
  0x432fffffffffffff,                                                         \
  /*  - The lowest-order mantissa bit has value 0.25.               */        \
  0x4310000000000000,                                                         \
  0x4310000000000001,                                                         \
  0x4310000000000002,                                                         \
  0x4310000000000003,                                                         \
  0x431fedcba9876543,                                                         \
  0x431ffffffffffffc,                                                         \
  0x431ffffffffffffd,                                                         \
  0x431ffffffffffffe,                                                         \
  0x431fffffffffffff,                                                         \
                                                                              \
  /* The same values again, but negated. */                                   \
  0xc330000000000000,                                                         \
  0xc330000000000001,                                                         \
  0xc330000000000002,                                                         \
  0xc330000000000003,                                                         \
  0xc33fedcba9876543,                                                         \
  0xc33ffffffffffffc,                                                         \
  0xc33ffffffffffffd,                                                         \
  0xc33ffffffffffffe,                                                         \
  0xc33fffffffffffff,                                                         \
  0xc320000000000000,                                                         \
  0xc320000000000001,                                                         \
  0xc320000000000002,                                                         \
  0xc320000000000003,                                                         \
  0xc32fedcba9876543,                                                         \
  0xc32ffffffffffffc,                                                         \
  0xc32ffffffffffffd,                                                         \
  0xc32ffffffffffffe,                                                         \
  0xc32fffffffffffff,                                                         \
  0xc310000000000000,                                                         \
  0xc310000000000001,                                                         \
  0xc310000000000002,                                                         \
  0xc310000000000003,                                                         \
  0xc31fedcba9876543,                                                         \
  0xc31ffffffffffffc,                                                         \
  0xc31ffffffffffffd,                                                         \
  0xc31ffffffffffffe,                                                         \
  0xc31fffffffffffff,                                                         \
                                                                              \
  /* Values relevant for conversions to integers (fcvt).    */                \
  0xc3e0000000000001,   /* The value just below INT64_MIN.          */        \
  0xc3e0000000000000,   /* INT64_MIN                                */        \
  0xc3dfffffffffffff,   /* The value just above INT64_MIN.          */        \
  0x43dfffffffffffff,   /* The value just below INT64_MAX.          */        \
                        /* INT64_MAX is not representable.          */        \
  0x43e0000000000000,   /* The value just above INT64_MAX.          */        \
                                                                              \
  0x43efffffffffffff,   /* The value just below UINT64_MAX.         */        \
                        /* UINT64_MAX is not representable.         */        \
  0x43f0000000000000,   /* The value just above UINT64_MAX.         */        \
                                                                              \
  0xc1e0000000200001,   /* The value just below INT32_MIN - 1.0.    */        \
  0xc1e0000000200000,   /* INT32_MIN - 1.0                          */        \
  0xc1e00000001fffff,   /* The value just above INT32_MIN - 1.0.    */        \
  0xc1e0000000100001,   /* The value just below INT32_MIN - 0.5.    */        \
  0xc1e0000000100000,   /* INT32_MIN - 0.5                          */        \
  0xc1e00000000fffff,   /* The value just above INT32_MIN - 0.5.    */        \
  0xc1e0000000000001,   /* The value just below INT32_MIN.          */        \
  0xc1e0000000000000,   /* INT32_MIN                                */        \
  0xc1dfffffffffffff,   /* The value just above INT32_MIN.          */        \
  0xc1dfffffffe00001,   /* The value just below INT32_MIN + 0.5.    */        \
  0xc1dfffffffe00000,   /* INT32_MIN + 0.5                          */        \
  0xc1dfffffffdfffff,   /* The value just above INT32_MIN + 0.5.    */        \
                                                                              \
  0x41dfffffff7fffff,   /* The value just below INT32_MAX - 1.0.    */        \
  0x41dfffffff800000,   /* INT32_MAX - 1.0                          */        \
  0x41dfffffff800001,   /* The value just above INT32_MAX - 1.0.    */        \
  0x41dfffffff9fffff,   /* The value just below INT32_MAX - 0.5.    */        \
  0x41dfffffffa00000,   /* INT32_MAX - 0.5                          */        \
  0x41dfffffffa00001,   /* The value just above INT32_MAX - 0.5.    */        \
  0x41dfffffffbfffff,   /* The value just below INT32_MAX.          */        \
  0x41dfffffffc00000,   /* INT32_MAX                                */        \
  0x41dfffffffc00001,   /* The value just above INT32_MAX.          */        \
  0x41dfffffffdfffff,   /* The value just below INT32_MAX + 0.5.    */        \
  0x41dfffffffe00000,   /* INT32_MAX + 0.5                          */        \
  0x41dfffffffe00001,   /* The value just above INT32_MAX + 0.5.    */        \
                                                                              \
  0x41efffffffbfffff,   /* The value just below UINT32_MAX - 1.0.   */        \
  0x41efffffffc00000,   /* UINT32_MAX - 1.0                         */        \
  0x41efffffffc00001,   /* The value just above UINT32_MAX - 1.0.   */        \
  0x41efffffffcfffff,   /* The value just below UINT32_MAX - 0.5.   */        \
  0x41efffffffd00000,   /* UINT32_MAX - 0.5                         */        \
  0x41efffffffd00001,   /* The value just above UINT32_MAX - 0.5.   */        \
  0x41efffffffdfffff,   /* The value just below UINT32_MAX.         */        \
  0x41efffffffe00000,   /* UINT32_MAX                               */        \
  0x41efffffffe00001,   /* The value just above UINT32_MAX.         */        \
  0x41efffffffefffff,   /* The value just below UINT32_MAX + 0.5.   */        \
  0x41effffffff00000,   /* UINT32_MAX + 0.5                         */        \
  0x41effffffff00001,   /* The value just above UINT32_MAX + 0.5.   */


// Float values, stored as uint32_t representations. This ensures exact bit
// representation, and avoids the loss of NaNs and suchlike through C++ casts.
#define INPUT_FLOAT_BASIC                                                     \
  /* Simple values. */                                                        \
  0x00000000,   /* 0.0                        */                              \
  0x00800000,   /* The smallest normal value. */                              \
  0x3effffff,   /* The value just below 0.5.  */                              \
  0x3f000000,   /* 0.5                        */                              \
  0x3f000001,   /* The value just above 0.5.  */                              \
  0x3f7fffff,   /* The value just below 1.0.  */                              \
  0x3f800000,   /* 1.0                        */                              \
  0x3f800001,   /* The value just above 1.0.  */                              \
  0x3fc00000,   /* 1.5                        */                              \
  0x41200000,   /* 10                         */                              \
  0x7f8fffff,   /* The largest finite value.  */                              \
                                                                              \
  /* Infinity. */                                                             \
  0x7f800000,                                                                 \
                                                                              \
  /* NaNs. */                                                                 \
  /*  - Quiet NaNs */                                                         \
  0x7fd23456,                                                                 \
  0x7fc00000,                                                                 \
  /*  - Signalling NaNs */                                                    \
  0x7f923456,                                                                 \
  0x7f800001,                                                                 \
                                                                              \
  /* Subnormals. */                                                           \
  /*  - A recognisable bit pattern. */                                        \
  0x00123456,                                                                 \
  /*  - The largest subnormal value. */                                       \
  0x007fffff,                                                                 \
  /*  - The smallest subnormal value. */                                      \
  0x00000001,                                                                 \
                                                                              \
  /* The same values again, but negated. */                                   \
  0x80000000,                                                                 \
  0x80800000,                                                                 \
  0xbeffffff,                                                                 \
  0xbf000000,                                                                 \
  0xbf000001,                                                                 \
  0xbf7fffff,                                                                 \
  0xbf800000,                                                                 \
  0xbf800001,                                                                 \
  0xbfc00000,                                                                 \
  0xc1200000,                                                                 \
  0xff8fffff,                                                                 \
  0xff800000,                                                                 \
  0xffd23456,                                                                 \
  0xffc00000,                                                                 \
  0xff923456,                                                                 \
  0xff800001,                                                                 \
  0x80123456,                                                                 \
  0x807fffff,                                                                 \
  0x80000001,


// Extra inputs. Passing these to 3- or 2-op instructions makes the trace file
// very large, so these should only be used with 1-op instructions.
#define INPUT_FLOAT_CONVERSIONS                                               \
  /* Values relevant for conversions to integers (frint).           */        \
  /*  - The lowest-order mantissa bit has value 1.                  */        \
  0x4b000000,                                                                 \
  0x4b000001,                                                                 \
  0x4b000002,                                                                 \
  0x4b000003,                                                                 \
  0x4b765432,                                                                 \
  0x4b7ffffc,                                                                 \
  0x4b7ffffd,                                                                 \
  0x4b7ffffe,                                                                 \
  0x4b7fffff,                                                                 \
  /*  - The lowest-order mantissa bit has value 0.5.                */        \
  0x4a800000,                                                                 \
  0x4a800001,                                                                 \
  0x4a800002,                                                                 \
  0x4a800003,                                                                 \
  0x4af65432,                                                                 \
  0x4afffffc,                                                                 \
  0x4afffffd,                                                                 \
  0x4afffffe,                                                                 \
  0x4affffff,                                                                 \
  /*  - The lowest-order mantissa bit has value 0.25.               */        \
  0x4a000000,                                                                 \
  0x4a000001,                                                                 \
  0x4a000002,                                                                 \
  0x4a000003,                                                                 \
  0x4a765432,                                                                 \
  0x4a7ffffc,                                                                 \
  0x4a7ffffd,                                                                 \
  0x4a7ffffe,                                                                 \
  0x4a7fffff,                                                                 \
                                                                              \
  /* The same values again, but negated. */                                   \
  0xcb000000,                                                                 \
  0xcb000001,                                                                 \
  0xcb000002,                                                                 \
  0xcb000003,                                                                 \
  0xcb765432,                                                                 \
  0xcb7ffffc,                                                                 \
  0xcb7ffffd,                                                                 \
  0xcb7ffffe,                                                                 \
  0xcb7fffff,                                                                 \
  0xca800000,                                                                 \
  0xca800001,                                                                 \
  0xca800002,                                                                 \
  0xca800003,                                                                 \
  0xcaf65432,                                                                 \
  0xcafffffc,                                                                 \
  0xcafffffd,                                                                 \
  0xcafffffe,                                                                 \
  0xcaffffff,                                                                 \
  0xca000000,                                                                 \
  0xca000001,                                                                 \
  0xca000002,                                                                 \
  0xca000003,                                                                 \
  0xca765432,                                                                 \
  0xca7ffffc,                                                                 \
  0xca7ffffd,                                                                 \
  0xca7ffffe,                                                                 \
  0xca7fffff,                                                                 \
                                                                              \
  /* Values relevant for conversions to integers (fcvt).            */        \
  0xdf000001,   /* The value just below INT64_MIN.                  */        \
  0xdf000000,   /* INT64_MIN                                        */        \
  0xdeffffff,   /* The value just above INT64_MIN.                  */        \
  0x5effffff,   /* The value just below INT64_MAX.                  */        \
                /* INT64_MAX is not representable.                  */        \
  0x5f000000,   /* The value just above INT64_MAX.                  */        \
                                                                              \
  0x5f7fffff,   /* The value just below UINT64_MAX.                 */        \
                /* UINT64_MAX is not representable.                 */        \
  0x5f800000,   /* The value just above UINT64_MAX.                 */        \
                                                                              \
  0xcf000001,   /* The value just below INT32_MIN.                  */        \
  0xcf000000,   /* INT32_MIN                                        */        \
  0xceffffff,   /* The value just above INT32_MIN.                  */        \
  0x4effffff,   /* The value just below INT32_MAX.                  */        \
                /* INT32_MAX is not representable.                  */        \
  0x4f000000,   /* The value just above INT32_MAX.                  */


// FP16 values, stored as uint16_t representations. This ensures exact bit
// representation, and avoids the loss of NaNs and suchlike through C++ casts.
#define INPUT_FLOAT16_BASIC                                                   \
  /* Simple values. */                                                        \
  0x0000,   /* 0.0                        */                                  \
  0x0400,   /* The smallest normal value. */                                  \
  0x37ff,   /* The value just below 0.5.  */                                  \
  0x3800,   /* 0.5                        */                                  \
  0x3801,   /* The value just above 0.5.  */                                  \
  0x3bff,   /* The value just below 1.0.  */                                  \
  0x3c00,   /* 1.0                        */                                  \
  0x3c01,   /* The value just above 1.0.  */                                  \
  0x3e00,   /* 1.5                        */                                  \
  0x4900,   /* 10                         */                                  \
  0x7bff,   /* The largest finite value.  */                                  \
                                                                              \
  /* Infinity. */                                                             \
  0x7c00,                                                                     \
                                                                              \
  /* NaNs. */                                                                 \
  /*  - Quiet NaNs */                                                         \
  0x7f23,                                                                     \
  0x7e00,                                                                     \
  /*  - Signalling NaNs */                                                    \
  0x7d23,                                                                     \
  0x7c01,                                                                     \
                                                                              \
  /* Subnormals. */                                                           \
  /*  - A recognisable bit pattern. */                                        \
  0x0012,                                                                     \
  /*  - The largest subnormal value. */                                       \
  0x03ff,                                                                     \
  /*  - The smallest subnormal value. */                                      \
  0x0001,                                                                     \
                                                                              \
  /* The same values again, but negated. */                                   \
  0x8000,                                                                     \
  0x8400,                                                                     \
  0xb7ff,                                                                     \
  0xb800,                                                                     \
  0xb801,                                                                     \
  0xbbff,                                                                     \
  0xbc00,                                                                     \
  0xbc01,                                                                     \
  0xbe00,                                                                     \
  0xc900,                                                                     \
  0xfbff,                                                                     \
  0xfc00,                                                                     \
  0xff23,                                                                     \
  0xfe00,                                                                     \
  0xfd23,                                                                     \
  0xfc01,                                                                     \
  0x8012,                                                                     \
  0x83ff,                                                                     \
  0x8001,


// Extra inputs. Passing these to 3- or 2-op instructions makes the trace file
// very large, so these should only be used with 1-op instructions. The largest
// normal FP16 value is 65504 which won't overflow int32_t or int64_t, so we
// don't test any cases like that.
#define INPUT_FLOAT16_CONVERSIONS                                             \
  /* Values relevant for conversions to integers (frint).           */        \
  /*  - The lowest-order mantissa bit has value 1.                  */        \
  0x6400,                                                                     \
  0x6401,                                                                     \
  0x6402,                                                                     \
  0x6403,                                                                     \
  0x6543,                                                                     \
  0x67fc,                                                                     \
  0x67fd,                                                                     \
  0x67fe,                                                                     \
  0x67ff,                                                                     \
  /*  - The lowest-order mantissa bit has value 0.5.                */        \
  0x6000,                                                                     \
  0x6001,                                                                     \
  0x6002,                                                                     \
  0x6003,                                                                     \
  0x6321,                                                                     \
  0x63fc,                                                                     \
  0x63fd,                                                                     \
  0x63fe,                                                                     \
  0x63ff,                                                                     \
  /*  - The lowest-order mantissa bit has value 0.25.               */        \
  0x5c00,                                                                     \
  0x5c01,                                                                     \
  0x5c02,                                                                     \
  0x5c03,                                                                     \
  0x5d32,                                                                     \
  0x5ffc,                                                                     \
  0x5ffd,                                                                     \
  0x5ffe,                                                                     \
  0x5fff,                                                                     \
                                                                              \
  /* The same values again, but negated. */                                   \
  0xe400,                                                                     \
  0xe401,                                                                     \
  0xe402,                                                                     \
  0xe403,                                                                     \
  0xe543,                                                                     \
  0xe7fc,                                                                     \
  0xe7fd,                                                                     \
  0xe7fe,                                                                     \
  0xe7ff,                                                                     \
  0xe000,                                                                     \
  0xe001,                                                                     \
  0xe002,                                                                     \
  0xe003,                                                                     \
  0xe321,                                                                     \
  0xe3fc,                                                                     \
  0xe3fd,                                                                     \
  0xe3fe,                                                                     \
  0xe3ff,                                                                     \
  0xdc00,                                                                     \
  0xdc01,                                                                     \
  0xdc02,                                                                     \
  0xdc03,                                                                     \
  0xdd32,                                                                     \
  0xdffc,                                                                     \
  0xdffd,                                                                     \
  0xdffe,                                                                     \
  0xdfff,                                                                     \
                                                                              \
  /* Some more NaN values. */                                                 \
  0x7c7f,                                                                     \
  0x7e91,                                                                     \
  0x7e00,                                                                     \
  0x7c91,                                                                     \
  0xfc7f,                                                                     \
  0xfe91,                                                                     \
  0xfe00,                                                                     \
  0xfc91,                                                                     \
  0xffff,

#define INPUT_16BITS_FIXEDPOINT_CONVERSIONS                                   \
  0x0000,                                                                     \
  0x0001,                                                                     \
  0x0400,                                                                     \
  0x0401,                                                                     \
  0x0476,                                                                     \
  0x0800,                                                                     \
  0x0801,                                                                     \
  0x0c00,                                                                     \
  0x0c01,                                                                     \
  0x1000,                                                                     \
  0x1001,                                                                     \
  0x1400,                                                                     \
  0x1401,                                                                     \
  0x1800,                                                                     \
  0x1c00,                                                                     \
  0x7f80,                                                                     \
  0x7fc0,                                                                     \
  0x7fff,                                                                     \
                                                                              \
  /* The same values again, but negated. */                                   \
  0x8000,                                                                     \
  0x8001,                                                                     \
  0x8400,                                                                     \
  0x8401,                                                                     \
  0x8476,                                                                     \
  0x8800,                                                                     \
  0x8801,                                                                     \
  0x8c00,                                                                     \
  0x8c01,                                                                     \
  0x9000,                                                                     \
  0x9001,                                                                     \
  0x9400,                                                                     \
  0x9401,                                                                     \
  0x9800,                                                                     \
  0x9c00,                                                                     \
  0xff80,                                                                     \
  0xffc0,                                                                     \
  0xffff

#define INPUT_32BITS_FIXEDPOINT_CONVERSIONS                                   \
  0x00000000,                                                                 \
  0x00000001,                                                                 \
  0x00800000,                                                                 \
  0x00800001,                                                                 \
  0x00876543,                                                                 \
  0x01000000,                                                                 \
  0x01000001,                                                                 \
  0x01800000,                                                                 \
  0x01800001,                                                                 \
  0x02000000,                                                                 \
  0x02000001,                                                                 \
  0x02800000,                                                                 \
  0x02800001,                                                                 \
  0x03000000,                                                                 \
  0x40000000,                                                                 \
  0x7fffff80,                                                                 \
  0x7fffffc0,                                                                 \
  0x7fffffff,                                                                 \
  0x80000000,                                                                 \
  0x80000100,                                                                 \
  0xffffff00,                                                                 \
  0xffffff80,                                                                 \
  0xffffffff,                                                                 \
  0xffffffff

#define INPUT_64BITS_FIXEDPOINT_CONVERSIONS                                   \
  0x0000000000000000,                                                         \
  0x0000000000000001,                                                         \
  0x0000000040000000,                                                         \
  0x0000000100000000,                                                         \
  0x4000000000000000,                                                         \
  0x4000000000000400,                                                         \
  0x000000007fffffff,                                                         \
  0x00000000ffffffff,                                                         \
  0x0000000080000000,                                                         \
  0x0000000080000001,                                                         \
  0x7ffffffffffffc00,                                                         \
  0x0123456789abcde0,                                                         \
  0x0000000012345678,                                                         \
  0xffffffffc0000000,                                                         \
  0xffffffff00000000,                                                         \
  0xc000000000000000,                                                         \
  0x1000000000000000,                                                         \
  0x1000000000000001,                                                         \
  0x1000000000000080,                                                         \
  0x1000000000000081,                                                         \
  0x1000000000000100,                                                         \
  0x1000000000000101,                                                         \
  0x1000000000000180,                                                         \
  0x1000000000000181,                                                         \
  0x1000000000000200,                                                         \
  0x1000000000000201,                                                         \
  0x1000000000000280,                                                         \
  0x1000000000000281,                                                         \
  0x1000000000000300,                                                         \
  0x8000000000000000,                                                         \
  0x8000000000000001,                                                         \
  0x8000000000000200,                                                         \
  0x8000000000000201,                                                         \
  0x8000000000000400,                                                         \
  0x8000000000000401,                                                         \
  0x8000000000000600,                                                         \
  0x8000000000000601,                                                         \
  0x8000000000000800,                                                         \
  0x8000000000000801,                                                         \
  0x8000000000000a00,                                                         \
  0x8000000000000a01,                                                         \
  0x8000000000000c00,                                                         \
  0x7ffffffffffffe00,                                                         \
  0x7fffffffffffffff,                                                         \
  0xfffffffffffffc00,                                                         \
  0xffffffffffffffff

// Some useful sets of values for testing vector SIMD operations.
#define INPUT_8BITS_IMM_LANECOUNT_FROMZERO                                    \
  0x00,                                                                       \
  0x01,                                                                       \
  0x02,                                                                       \
  0x03,                                                                       \
  0x04,                                                                       \
  0x05,                                                                       \
  0x06,                                                                       \
  0x07,                                                                       \
  0x08,                                                                       \
  0x09,                                                                       \
  0x0a,                                                                       \
  0x0b,                                                                       \
  0x0c,                                                                       \
  0x0d,                                                                       \
  0x0e,                                                                       \
  0x0f

#define INPUT_16BITS_IMM_LANECOUNT_FROMZERO                                    \
  0x00,                                                                       \
  0x01,                                                                       \
  0x02,                                                                       \
  0x03,                                                                       \
  0x04,                                                                       \
  0x05,                                                                       \
  0x06,                                                                       \
  0x07

#define INPUT_32BITS_IMM_LANECOUNT_FROMZERO                                    \
  0x00,                                                                       \
  0x01,                                                                       \
  0x02,                                                                       \
  0x03

#define INPUT_64BITS_IMM_LANECOUNT_FROMZERO                                    \
  0x00,                                                                       \
  0x01

#define INPUT_8BITS_IMM_TYPEWIDTH_BASE                                        \
  0x01,                                                                       \
  0x02,                                                                       \
  0x03,                                                                       \
  0x04,                                                                       \
  0x05,                                                                       \
  0x06,                                                                       \
  0x07

#define INPUT_16BITS_IMM_TYPEWIDTH_BASE                                       \
  INPUT_8BITS_IMM_TYPEWIDTH_BASE,                                             \
  0x08,                                                                       \
  0x09,                                                                       \
  0x0a,                                                                       \
  0x0b,                                                                       \
  0x0c,                                                                       \
  0x0d,                                                                       \
  0x0e,                                                                       \
  0x0f

#define INPUT_32BITS_IMM_TYPEWIDTH_BASE                                       \
  INPUT_16BITS_IMM_TYPEWIDTH_BASE,                                            \
  0x10,                                                                       \
  0x11,                                                                       \
  0x12,                                                                       \
  0x13,                                                                       \
  0x14,                                                                       \
  0x15,                                                                       \
  0x16,                                                                       \
  0x17,                                                                       \
  0x18,                                                                       \
  0x19,                                                                       \
  0x1a,                                                                       \
  0x1b,                                                                       \
  0x1c,                                                                       \
  0x1d,                                                                       \
  0x1e,                                                                       \
  0x1f

#define INPUT_64BITS_IMM_TYPEWIDTH_BASE                                       \
  INPUT_32BITS_IMM_TYPEWIDTH_BASE,                                            \
  0x20,                                                                       \
  0x21,                                                                       \
  0x22,                                                                       \
  0x23,                                                                       \
  0x24,                                                                       \
  0x25,                                                                       \
  0x26,                                                                       \
  0x27,                                                                       \
  0x28,                                                                       \
  0x29,                                                                       \
  0x2a,                                                                       \
  0x2b,                                                                       \
  0x2c,                                                                       \
  0x2d,                                                                       \
  0x2e,                                                                       \
  0x2f,                                                                       \
  0x30,                                                                       \
  0x31,                                                                       \
  0x32,                                                                       \
  0x33,                                                                       \
  0x34,                                                                       \
  0x35,                                                                       \
  0x36,                                                                       \
  0x37,                                                                       \
  0x38,                                                                       \
  0x39,                                                                       \
  0x3a,                                                                       \
  0x3b,                                                                       \
  0x3c,                                                                       \
  0x3d,                                                                       \
  0x3e,                                                                       \
  0x3f

#define INPUT_8BITS_IMM_TYPEWIDTH                                             \
  INPUT_8BITS_IMM_TYPEWIDTH_BASE,                                             \
  0x08

#define INPUT_16BITS_IMM_TYPEWIDTH                                            \
  INPUT_16BITS_IMM_TYPEWIDTH_BASE,                                            \
  0x10

#define INPUT_32BITS_IMM_TYPEWIDTH                                            \
  INPUT_32BITS_IMM_TYPEWIDTH_BASE,                                            \
  0x20

#define INPUT_64BITS_IMM_TYPEWIDTH                                            \
  INPUT_64BITS_IMM_TYPEWIDTH_BASE,                                            \
  0x40

#define INPUT_8BITS_IMM_TYPEWIDTH_FROMZERO                                    \
  0x00,                                                                       \
  INPUT_8BITS_IMM_TYPEWIDTH_BASE

#define INPUT_16BITS_IMM_TYPEWIDTH_FROMZERO                                   \
  0x00,                                                                       \
  INPUT_16BITS_IMM_TYPEWIDTH_BASE

#define INPUT_32BITS_IMM_TYPEWIDTH_FROMZERO                                   \
  0x00,                                                                       \
  INPUT_32BITS_IMM_TYPEWIDTH_BASE

#define INPUT_64BITS_IMM_TYPEWIDTH_FROMZERO                                   \
  0x00,                                                                       \
  INPUT_64BITS_IMM_TYPEWIDTH_BASE

#define INPUT_16BITS_IMM_TYPEWIDTH_FROMZERO_TOWIDTH                           \
  0x00,                                                                       \
  INPUT_16BITS_IMM_TYPEWIDTH_BASE,                                            \
  0x10

#define INPUT_32BITS_IMM_TYPEWIDTH_FROMZERO_TOWIDTH                           \
  0x00,                                                                       \
  INPUT_32BITS_IMM_TYPEWIDTH_BASE,                                            \
  0x20

#define INPUT_64BITS_IMM_TYPEWIDTH_FROMZERO_TOWIDTH                           \
  0x00,                                                                       \
  INPUT_64BITS_IMM_TYPEWIDTH_BASE,                                            \
  0x40

#define INPUT_8BITS_BASIC                                                     \
  0x00,                                                                       \
  0x01,                                                                       \
  0x02,                                                                       \
  0x08,                                                                       \
  0x33,                                                                       \
  0x55,                                                                       \
  0x7d,                                                                       \
  0x7e,                                                                       \
  0x7f,                                                                       \
  0x80,                                                                       \
  0x81,                                                                       \
  0x82,                                                                       \
  0x83,                                                                       \
  0xaa,                                                                       \
  0xcc,                                                                       \
  0xf8,                                                                       \
  0xfd,                                                                       \
  0xfe,                                                                       \
  0xff

// Basic values for vector SIMD operations of types 4H or 8H.
#define INPUT_16BITS_BASIC                                                    \
  0x0000,                                                                     \
  0x0001,                                                                     \
  0x0002,                                                                     \
  0x0010,                                                                     \
  0x007d,                                                                     \
  0x007e,                                                                     \
  0x007f,                                                                     \
  0x3333,                                                                     \
  0x5555,                                                                     \
  0x7ffd,                                                                     \
  0x7ffe,                                                                     \
  0x7fff,                                                                     \
  0x8000,                                                                     \
  0x8001,                                                                     \
  0xaaaa,                                                                     \
  0xcccc,                                                                     \
  0xff80,                                                                     \
  0xff81,                                                                     \
  0xff82,                                                                     \
  0xff83,                                                                     \
  0xfff0,                                                                     \
  0xfffd,                                                                     \
  0xfffe,                                                                     \
  0xffff

// Basic values for vector SIMD operations of types 2S or 4S.
#define INPUT_32BITS_BASIC                                                    \
  0x00000000,                                                                 \
  0x00000001,                                                                 \
  0x00000002,                                                                 \
  0x00000020,                                                                 \
  0x0000007d,                                                                 \
  0x0000007e,                                                                 \
  0x0000007f,                                                                 \
  0x00007ffd,                                                                 \
  0x00007ffe,                                                                 \
  0x00007fff,                                                                 \
  0x33333333,                                                                 \
  0x55555555,                                                                 \
  0x7ffffffd,                                                                 \
  0x7ffffffe,                                                                 \
  0x7fffffff,                                                                 \
  0x80000000,                                                                 \
  0x80000001,                                                                 \
  0xaaaaaaaa,                                                                 \
  0xcccccccc,                                                                 \
  0xffff8000,                                                                 \
  0xffff8001,                                                                 \
  0xffff8002,                                                                 \
  0xffff8003,                                                                 \
  0xffffff80,                                                                 \
  0xffffff81,                                                                 \
  0xffffff82,                                                                 \
  0xffffff83,                                                                 \
  0xffffffe0,                                                                 \
  0xfffffffd,                                                                 \
  0xfffffffe,                                                                 \
  0xffffffff

// Basic values for vector SIMD operations of type 2D
#define INPUT_64BITS_BASIC                                                    \
  0x0000000000000000,                                                         \
  0x0000000000000001,                                                         \
  0x0000000000000002,                                                         \
  0x0000000000000040,                                                         \
  0x000000000000007d,                                                         \
  0x000000000000007e,                                                         \
  0x000000000000007f,                                                         \
  0x0000000000007ffd,                                                         \
  0x0000000000007ffe,                                                         \
  0x0000000000007fff,                                                         \
  0x000000007ffffffd,                                                         \
  0x000000007ffffffe,                                                         \
  0x000000007fffffff,                                                         \
  0x3333333333333333,                                                         \
  0x5555555555555555,                                                         \
  0x7ffffffffffffffd,                                                         \
  0x7ffffffffffffffe,                                                         \
  0x7fffffffffffffff,                                                         \
  0x8000000000000000,                                                         \
  0x8000000000000001,                                                         \
  0x8000000000000002,                                                         \
  0x8000000000000003,                                                         \
  0xaaaaaaaaaaaaaaaa,                                                         \
  0xcccccccccccccccc,                                                         \
  0xffffffff80000000,                                                         \
  0xffffffff80000001,                                                         \
  0xffffffff80000002,                                                         \
  0xffffffff80000003,                                                         \
  0xffffffffffff8000,                                                         \
  0xffffffffffff8001,                                                         \
  0xffffffffffff8002,                                                         \
  0xffffffffffff8003,                                                         \
  0xffffffffffffff80,                                                         \
  0xffffffffffffff81,                                                         \
  0xffffffffffffff82,                                                         \
  0xffffffffffffff83,                                                         \
  0xffffffffffffffc0,                                                         \
  0xfffffffffffffffd,                                                         \
  0xfffffffffffffffe,                                                         \
  0xffffffffffffffff

// clang-format on

// For most 2- and 3-op instructions, use only basic inputs. Because every
// combination is tested, the length of the output trace is very sensitive to
// the length of this list.
static const uint64_t kInputDoubleBasic[] = {INPUT_DOUBLE_BASIC};
static const uint32_t kInputFloatBasic[] = {INPUT_FLOAT_BASIC};
static const uint16_t kInputFloat16Basic[] = {INPUT_FLOAT16_BASIC};

// TODO: Define different values when the traces file is split.
#define INPUT_DOUBLE_ACC_DESTINATION INPUT_DOUBLE_BASIC
#define INPUT_FLOAT_ACC_DESTINATION INPUT_FLOAT_BASIC
#define INPUT_FLOAT16_ACC_DESTINATION INPUT_FLOAT16_BASIC

static const uint64_t kInputDoubleAccDestination[] = {
    INPUT_DOUBLE_ACC_DESTINATION};

static const uint32_t kInputFloatAccDestination[] = {
    INPUT_FLOAT_ACC_DESTINATION};

static const uint16_t kInputFloat16AccDestination[] = {
    INPUT_FLOAT16_ACC_DESTINATION};

// For conversions, include several extra inputs.
static const uint64_t kInputDoubleConversions[] = {
    INPUT_DOUBLE_BASIC INPUT_DOUBLE_CONVERSIONS};

static const uint32_t kInputFloatConversions[] = {
    INPUT_FLOAT_BASIC INPUT_FLOAT_CONVERSIONS};

static const uint64_t kInput64bitsFixedPointConversions[] =
    {INPUT_64BITS_BASIC, INPUT_64BITS_FIXEDPOINT_CONVERSIONS};

static const uint32_t kInput32bitsFixedPointConversions[] =
    {INPUT_32BITS_BASIC, INPUT_32BITS_FIXEDPOINT_CONVERSIONS};

static const uint16_t kInput16bitsFixedPointConversions[] =
    {INPUT_16BITS_BASIC, INPUT_16BITS_FIXEDPOINT_CONVERSIONS};

static const uint16_t kInputFloat16Conversions[] = {
    INPUT_FLOAT16_BASIC INPUT_FLOAT16_CONVERSIONS};

static const uint8_t kInput8bitsBasic[] = {INPUT_8BITS_BASIC};

static const uint16_t kInput16bitsBasic[] = {INPUT_16BITS_BASIC};

static const uint32_t kInput32bitsBasic[] = {INPUT_32BITS_BASIC};

static const uint64_t kInput64bitsBasic[] = {INPUT_64BITS_BASIC};

static const int kInput8bitsImmTypeWidth[] = {INPUT_8BITS_IMM_TYPEWIDTH};

static const int kInput16bitsImmTypeWidth[] = {INPUT_16BITS_IMM_TYPEWIDTH};

static const int kInput32bitsImmTypeWidth[] = {INPUT_32BITS_IMM_TYPEWIDTH};

static const int kInput64bitsImmTypeWidth[] = {INPUT_64BITS_IMM_TYPEWIDTH};

static const int kInput8bitsImmTypeWidthFromZero[] = {
    INPUT_8BITS_IMM_TYPEWIDTH_FROMZERO};

static const int kInput16bitsImmTypeWidthFromZero[] = {
    INPUT_16BITS_IMM_TYPEWIDTH_FROMZERO};

static const int kInput32bitsImmTypeWidthFromZero[] = {
    INPUT_32BITS_IMM_TYPEWIDTH_FROMZERO};

static const int kInput64bitsImmTypeWidthFromZero[] = {
    INPUT_64BITS_IMM_TYPEWIDTH_FROMZERO};

static const int kInput16bitsImmTypeWidthFromZeroToWidth[] = {
    INPUT_16BITS_IMM_TYPEWIDTH_FROMZERO_TOWIDTH};

static const int kInput32bitsImmTypeWidthFromZeroToWidth[] = {
    INPUT_32BITS_IMM_TYPEWIDTH_FROMZERO_TOWIDTH};

static const int kInput64bitsImmTypeWidthFromZeroToWidth[] = {
    INPUT_64BITS_IMM_TYPEWIDTH_FROMZERO_TOWIDTH};

// These immediate values are used only in 'shll{2}' tests.
static const int kInput8bitsImmSHLL[] = {8};
static const int kInput16bitsImmSHLL[] = {16};
static const int kInput32bitsImmSHLL[] = {32};

static const double kInputDoubleImmZero[] = {0.0};

static const int kInput8bitsImmZero[] = {0};

static const int kInput16bitsImmZero[] = {0};

static const int kInput32bitsImmZero[] = {0};

static const int kInput64bitsImmZero[] = {0};

static const int kInput8bitsImmLaneCountFromZero[] = {
    INPUT_8BITS_IMM_LANECOUNT_FROMZERO};

static const int kInput16bitsImmLaneCountFromZero[] = {
    INPUT_16BITS_IMM_LANECOUNT_FROMZERO};

static const int kInput32bitsImmLaneCountFromZero[] = {
    INPUT_32BITS_IMM_LANECOUNT_FROMZERO};

static const int kInput64bitsImmLaneCountFromZero[] = {
    INPUT_64BITS_IMM_LANECOUNT_FROMZERO};

// TODO: Define different values when the traces file is split.
#define INPUT_8BITS_ACC_DESTINATION INPUT_8BITS_BASIC
#define INPUT_16BITS_ACC_DESTINATION INPUT_16BITS_BASIC
#define INPUT_32BITS_ACC_DESTINATION INPUT_32BITS_BASIC
#define INPUT_64BITS_ACC_DESTINATION INPUT_64BITS_BASIC

static const uint8_t kInput8bitsAccDestination[] = {
    INPUT_8BITS_ACC_DESTINATION};

static const uint16_t kInput16bitsAccDestination[] = {
    INPUT_16BITS_ACC_DESTINATION};

static const uint32_t kInput32bitsAccDestination[] = {
    INPUT_32BITS_ACC_DESTINATION};

static const uint64_t kInput64bitsAccDestination[] = {
    INPUT_64BITS_ACC_DESTINATION};

static const int kInputHIndices[] = {0, 1, 2, 3, 4, 5, 6, 7};

static const int kInputSIndices[] = {0, 1, 2, 3};

static const int kInputDIndices[] = {0, 1};
