/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/client/EarlyDataRejectionPolicy.h>

namespace fizz {
namespace client {

static bool certIdentityMatch(const Cert* a, const Cert* b) {
  if (!a || !b) {
    return a == b;
  }

  return a->getIdentity() == b->getIdentity();
}

bool earlyParametersMatch(const State& state) {
  if (*state.version() != state.earlyDataParams()->version) {
    // Temporarily allow early data retry when transitioning from tls_1_3_26_fb
    // to the RFC version, to make the transition less disruptful. This is safe
    // as the versions are identical except for the ProtocolVersion.
    if (*state.version() != ProtocolVersion::tls_1_3 ||
        state.earlyDataParams()->version != ProtocolVersion::tls_1_3_26_fb) {
      return false;
    }
  }

  if (*state.cipher() != state.earlyDataParams()->cipher) {
    return false;
  }

  if (state.alpn() != state.earlyDataParams()->alpn) {
    return false;
  }

  if (!certIdentityMatch(
          state.serverCert().get(),
          state.earlyDataParams()->serverCert.get())) {
    return false;
  }

  if (!certIdentityMatch(
          state.clientCert().get(),
          state.earlyDataParams()->clientCert.get())) {
    return false;
  }

  return true;
}
} // namespace client
} // namespace fizz
