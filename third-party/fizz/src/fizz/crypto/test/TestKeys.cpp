/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/test/TestKeys.h>

// @lint-ignore-every PRIVATEKEY

namespace fizz {
namespace test {

template <>
KeyParams getKeyParams<fizz::P256>() {
  return KeyParams{
      kP256Key,
      kP256K1Key,
      "048d5e897c896b17e1679766c14c785dd2c328c3"
      "eecc7dbfd2e2e817cd35c786aceea79bf1286ab8"
      "a5c3c464c46f5ba06338b24ea96ce442a4d13356"
      "902dfcd1e9",
      "048d5e897c896b17e1679766c14c785dd2c328c3"
      "eecc7dbfd2e2e817cd35c786abeea79bf1286ab8"
      "a5c3c464c46f5ba06338b24ea96ce442a4d13356"
      "902dfcd1e9",
      "048d5e897c896b17e1679766c14c785dd2c328c3"};
}

template <>
KeyParams getKeyParams<fizz::P384>() {
  return KeyParams{
      kP384Key,
      kP256Key,
      "04811041962bbac9859eb2aa19a475"
      "2e573949d3f1eb8abcb131c1674e75"
      "061f5e17b6a24724eeb73268ed82fd"
      "3d2bfc93db88cabbf8c9da9e4c1479"
      "ffdb9a54b522990aef401c1ab004aa"
      "1fdf0e26aa8692ca5860ad6f6cf8ea"
      "3bb95dbecb82a3",
      "04811041962bbac9859eb2aa19a475"
      "2e573949d3f1eb8abcb131c1674e75"
      "061f5e17b6a24724eeb73268ed82fd"
      "3d2bfc93db88cabbf8c9da9e4c1479"
      "ffdb9a54b522990aef401c1ab004aa"
      "1fdf0e26aa8621ca5860ad6f6cf8ea"
      "3bb95dbecb82a3",
      "3bb95dbecb82a3"};
}

template <>
KeyParams getKeyParams<fizz::P521>() {
  return KeyParams{
      kP521Key,
      kP384Key,
      "0401a76d5b0c42f557e418bf982b35"
      "6e903f937b9d530cca8250e07c98ca"
      "03293958b2fa8da20f9c4e9f7684df"
      "3e872b470dac5368697b664ce4c677"
      "3feb8862707b5f01434182eb962a3a"
      "a8a707c868caa6edee62dd8456172e"
      "f0bd738fac3abb04e87bd3e9ea9c1b"
      "d09f8720a994c6162cb42546f86e32"
      "8de930387e8ecc2c68ec41bcf2",
      "0401a76d5b0c42f557e418bf982b35"
      "6e903f937b9d530cca8250e07c98bd"
      "03293958b2fa8da20f9c4e9f7684df"
      "3e872b470dac5368697b664ce4c677"
      "3feb8862707b5f01434182eb962a3a"
      "a8a707c868caa6edee62dd8456172e"
      "f0bd738fac3abb04e87bd3e9ea9c1b"
      "d09f8720a994c6162cb42546f86e32"
      "8de930387e8ecc2c68ec41bcf2",
      "8de930387e8ecc2c68ec41bcf2"};
}
} // namespace test
} // namespace fizz
