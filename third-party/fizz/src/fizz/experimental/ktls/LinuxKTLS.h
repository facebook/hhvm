/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

/**
 * This file serves as a portability shim to #include'ing <linux/tls.h>
 *
 * Certain build environments may be using a kernel that supports kTLS but
 * lacks the corresponding uapi headers (or are outdated). This is frequently
 * the case if using distro supplied linux uapi headers, but running a
 * non-distro (or backported) kernel.
 *
 * Fizz relies on runtime detection (through `setsockopt()`) to detect kTLS
 * availability.
 *
 * These values are safe to duplicate; they will not change since they are part
 * of the user-kernel ABI.
 *
 * The first kernel to contain kTLS contained a linux/tls.h uapi header that
 *    1) Only supported TLS_TX
 *    2) Only suported TLS 1.2
 *    3) Only supported AES-128-GCM
 */

#include <folly/CPortability.h>

#if defined(__linux__) && !FOLLY_MOBILE
#include <linux/version.h>

// Minimum version of linux uapi headers we require; this is when linux/tls.h
// was introduced
#if LINUX_VERSION_CODE < 265216
#define FIZZ_PLATFORM_CAPABLE_KTLS 0
#else
#define FIZZ_PLATFORM_CAPABLE_KTLS 1
#endif
#else
#define FIZZ_PLATFORM_CAPABLE_KTLS 0
#endif

#if FIZZ_PLATFORM_CAPABLE_KTLS
#include <linux/tls.h>

#ifndef TLS_TX
#define TLS_TX 1
#endif

#ifndef TLS_RX
#define TLS_RX 2
#endif

#ifndef TLS_TX_ZEROCOPY_RO
#define TLS_TX_ZEROCOPY_RO 3
#endif

#ifndef TLS_RX_EXPECT_NO_PAD
#define TLS_RX_EXPECT_NO_PAD 4
#endif

#ifndef TLS_1_3_VERSION_MAJOR
#define TLS_1_3_VERSION_MAJOR 0x3
#endif

#ifndef TLS_1_3_VERSION_MINOR
#define TLS_1_3_VERSION_MINOR 0x4
#endif

#ifndef TLS_1_3_VERSION
#define TLS_1_3_VERSION TLS_VERSION_NUMBER(TLS_1_3)
#endif

#ifndef TLS_SET_RECORD_TYPE
#define TLS_SET_RECORD_TYPE 1
#endif

#ifndef TLS_GET_RECORD_TYPE
#define TLS_GET_RECORD_TYPE 2
#endif

#ifndef TLS_CIPHER_AES_GCM_256
#define TLS_CIPHER_AES_GCM_256 52
#endif

#ifndef TLS_CIPHER_AES_GCM_256_IV_SIZE
#define TLS_CIPHER_AES_GCM_256_IV_SIZE 8
#endif

#ifndef TLS_CIPHER_AES_GCM_256_KEY_SIZE
#define TLS_CIPHER_AES_GCM_256_KEY_SIZE 32
#endif

#ifndef TLS_CIPHER_AES_GCM_256_SALT_SIZE
#define TLS_CIPHER_AES_GCM_256_SALT_SIZE 4
#endif

#ifndef TLS_CIPHER_AES_GCM_256_TAG_SIZE
#define TLS_CIPHER_AES_GCM_256_TAG_SIZE 16
#endif

#ifndef TLS_CIPHER_AES_GCM_256_REC_SEQ_SIZE
#define TLS_CIPHER_AES_GCM_256_REC_SEQ_SIZE 8
#endif

#endif
