/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/fizz-config.h>

#if FIZZ_HAVE_LIBAEGIS
namespace fizz::libaegis {

template <class T>
Status makeCipher(std::unique_ptr<Aead>& ret, Error& err);

template <>
Status makeCipher<fizz::AEGIS128L>(std::unique_ptr<Aead>& ret, Error& err);
template <>
Status makeCipher<fizz::AEGIS256>(std::unique_ptr<Aead>& ret, Error& err);
} // namespace fizz::libaegis
#endif
