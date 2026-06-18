/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/String.h>

// @lint-ignore-every PRIVATEKEY

namespace fizz {
namespace extensions {
namespace test {

// Hex-encoded delegated credential extension (P256) paired with
// kDelegationUsageCert.
constexpr folly::StringPiece kDelegatedCred{
    "001ce0d1040300005b3059301306072a8648ce3d020106082a8648ce3d030107"
    "03420004db94b7b305323633ccc5a5f12a3b07c22bbf86e5d531ed94d09c5bfe"
    "860b72e5dc73b8267729f34150a6422cbdc87484a535125ff3a02a03372c0969"
    "3abf050504030048304602210089a87271db53ca89f4eccbc37e616df92f4b35"
    "1f5774c56da74bacfc2774e434022100af1d4561dab905345344475a33c8f132"
    "0d82978beae2fca0f34b8d40713b92ac"};

// Leaf cert carrying the DelegationUsage extension (expires 2020-05-23).
constexpr folly::StringPiece kDelegationUsageCert = R"(
-----BEGIN CERTIFICATE-----
MIIB6TCCAY+gAwIBAgIJAKlQpSahHUIWMAoGCCqGSM49BAMCMEIxCzAJBgNVBAYT
AlhYMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t
cGFueSBMdGQwHhcNMTkwNTI0MTk1MjU3WhcNMjAwNTIzMTk1MjU3WjBCMQswCQYD
VQQGEwJYWDEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0
IENvbXBhbnkgTHRkMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE8RC+4O48vtBh
JhSn3/wuzfygo/AQGGNMavAb5YnZpy6rMaY9UG3OlFfkRRmvETlbn3CXD0klXuc/
wYCKoVXGYqNuMGwwHQYDVR0OBBYEFC892QWimVBjX1AODjjL+SqTN1meMB8GA1Ud
IwQYMBaAFC892QWimVBjX1AODjjL+SqTN1meMAwGA1UdEwQFMAMBAf8wCwYDVR0P
BAQDAgHmMA8GCSsGAQQBgtpLLAQCBQAwCgYIKoZIzj0EAwIDSAAwRQIhAPoWbJWf
Fw+uQ6c27kul/uTNIF4GOEUCmCWVvc6qkhHVAiBKTBrUi8h8g/U0yQ4prS0/wfkw
FghrPnYCODq235mY2A==
-----END CERTIFICATE-----
)";

/*
 *  Self-signed delegation certificate (CN=revproxy-delegated-ec, expires 2119)
 *  paired with kP256CredCertKey.
 *  Prerequisites:
 *    - P133567922 in config.cfg
 *    - kP256CredCertKey in p256_key.pem
 *  Command: openssl req -new -key p256_key.pem -x509 -nodes -days 365 -config
 * config.cfg
 */
constexpr folly::StringPiece kP256CredCert = R"(
-----BEGIN CERTIFICATE-----
MIICKzCCAdGgAwIBAgIJAPi2vMRfOVd0MAoGCCqGSM49BAMCMGIxCzAJBgNVBAYT
AlhYMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t
cGFueSBMdGQxHjAcBgNVBAMMFXJldnByb3h5LWRlbGVnYXRlZC1lYzAgFw0xOTA5
MjMwMjAyMzVaGA8yMTE5MDgzMDAyMDIzNVowYjELMAkGA1UEBhMCWFgxFTATBgNV
BAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UECgwTRGVmYXVsdCBDb21wYW55IEx0ZDEe
MBwGA1UEAwwVcmV2cHJveHktZGVsZWdhdGVkLWVjMFkwEwYHKoZIzj0CAQYIKoZI
zj0DAQcDQgAE7EbZMKds65EYciaSULFH4wZKt/OThiUL4uQW9cybr2HIzK68corO
JCeHXOsV3lpYS46b39SBZr1GZprFHH5gHaNuMGwwHQYDVR0OBBYEFMLkRMB4SclK
8K8uYMQBaYw0gNP7MB8GA1UdIwQYMBaAFMLkRMB4SclK8K8uYMQBaYw0gNP7MAwG
A1UdEwQFMAMBAf8wCwYDVR0PBAQDAgHmMA8GCSsGAQQBgtpLLAQCBQAwCgYIKoZI
zj0EAwIDSAAwRQIgB2EWbwWohYziQ2LmY8Qmn8y0WKR6Mbm5aad0rUBvtK4CIQCv
0U6Z/gFrVr0Cb2kc7M37KD9z5eeTwkQuGqs5GXF8Ow==
-----END CERTIFICATE-----
)";

/*
 *  Randomly generated ECDSA-P256 private key for kP256CredCert.
 *  Command: openssl ecparam -name secp256r1 -genkey
 */
constexpr folly::StringPiece kP256CredCertKey = R"(
-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgOe/v6hxwTP9uA5dE
se5CO6ARqeOYXEy1ede9eRCmDduhRANCAATsRtkwp2zrkRhyJpJQsUfjBkq385OG
JQvi5Bb1zJuvYcjMrrxyis4kJ4dc6xXeWlhLjpvf1IFmvUZmmsUcfmAd
-----END PRIVATE KEY-----
)";

} // namespace test
} // namespace extensions
} // namespace fizz
