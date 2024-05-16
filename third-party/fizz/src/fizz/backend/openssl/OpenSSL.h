// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

/*

This is the backend for the openssl crypto implementations.
Include this file to use openssl features.

*/

#include <fizz/fizz-config.h>

#include <fizz/backend/openssl/OpenSSLFactory.h>
#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/backend/openssl/certificate/OpenSSLPeerCertImpl.h>
#include <fizz/backend/openssl/certificate/OpenSSLSelfCertImpl.h>
#include <fizz/backend/openssl/crypto/ECCurve.h>
#include <fizz/backend/openssl/crypto/OpenSSLKeyUtils.h>
#include <fizz/backend/openssl/crypto/Sha256.h>
#include <fizz/backend/openssl/crypto/Sha384.h>
#include <fizz/backend/openssl/crypto/Sha512.h>
#include <fizz/backend/openssl/crypto/aead/AESGCM128.h>
#include <fizz/backend/openssl/crypto/aead/AESGCM256.h>
#include <fizz/backend/openssl/crypto/aead/AESOCB128.h>
#include <fizz/backend/openssl/crypto/aead/ChaCha20Poly1305.h>
#include <fizz/backend/openssl/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/backend/openssl/crypto/exchange/OpenSSLKeyExchange.h>
#include <fizz/backend/openssl/crypto/signature/Signature.h>
#include <folly/io/IOBuf.h>
