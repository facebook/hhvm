// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <openssl/opensslv.h>

#define FIZZ_OPENSSL_HAS_ED25519 (OPENSSL_VERSION_NUMBER >= 0x10101000L)
