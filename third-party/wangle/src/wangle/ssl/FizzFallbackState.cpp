/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include "wangle/ssl/FizzFallbackState.h"

namespace {

#if FOLLY_OPENSSL_PREREQ(3, 0, 0)
using DupFuncFromDType = void**;
#else
using DupFuncFromDType = void*;
#endif

static void freeFunc(
    void* /* parent */,
    void* ptr,
    CRYPTO_EX_DATA* /* ad */,
    int /* idx */,
    long /* argl */,
    void* /* argp */) {
  delete static_cast<wangle::FizzFallbackState*>(ptr);
}

static int dupFunc(
    CRYPTO_EX_DATA* to,
    const CRYPTO_EX_DATA* from,
    DupFuncFromDType from_d,
    int idx,
    long /* argl */,
    void* /* argp */) {
  void** pdata = (void**)from_d;
  wangle::FizzFallbackState* prevState = (wangle::FizzFallbackState*)*pdata;
  if (!prevState) {
    // no data to copy
    return 1;
  }
  auto newState = std::make_unique<wangle::FizzFallbackState>(*prevState);
  if (CRYPTO_set_ex_data(to, idx, newState.get()) == 1) {
    newState.release();
    return 1;
  }
  return 0;
}

static int index() {
  static int index =
      SSL_get_ex_new_index(0, nullptr, nullptr, dupFunc, freeFunc);
  return index;
}
} // namespace

namespace wangle {

void FizzFallbackState::attachToSSL(
    std::unique_ptr<FizzFallbackState> state,
    SSL* ssl) {
  if (SSL_set_ex_data(ssl, index(), state.get()) == 1) {
    state.release();
  }
}

FizzFallbackState* FizzFallbackState::tryFromSSL(SSL* ssl) {
  auto found = SSL_get_ex_data(ssl, index());
  if (found) {
    return static_cast<FizzFallbackState*>(found);
  }
  return nullptr;
}
} // namespace wangle
