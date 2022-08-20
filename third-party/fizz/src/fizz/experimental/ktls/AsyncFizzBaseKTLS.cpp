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

#include <fizz/client/State.h>
#include <fizz/experimental/ktls/AsyncFizzBaseKTLS.h>
#include <fizz/record/Extensions.h>

namespace fizz {
namespace detail {

using fizz::client::CachedPsk;
using fizz::client::PskCache;

static void setPskAndSecret(
    KeyScheduler& ks,
    CachedPsk& psk,
    NewSessionTicket& nst,
    folly::ByteRange resumptionSecret) {
  auto derivedResumptionSecret =
      ks.getResumptionSecret(resumptionSecret, nst.ticket_nonce->coalesce());

  auto pskRange = nst.ticket->coalesce();
  auto secretRange = derivedResumptionSecret->coalesce();
  psk.psk = std::string(pskRange.begin(), pskRange.end());
  psk.secret = std::string(secretRange.begin(), secretRange.end());
}

static uint32_t getMaxEarlyDataSize(const NewSessionTicket& nst) {
  auto earlyData = getExtension<TicketEarlyData>(nst.extensions);
  if (earlyData) {
    return earlyData->max_early_data_size;
  } else {
    return 0;
  }
}

KTLSCallbackImpl::TicketHandler makeTicketHandler(
    std::string&& pskIdentity,
    const fizz::client::State& state,
    std::shared_ptr<PskCache>&& pskCache) {
  return [pskIdentity = std::move(pskIdentity),
          pskCache = std::move(pskCache),
          resumptionSecret = state.resumptionSecret()->clone(),
          version = *state.version(),
          cipher = *state.cipher(),
          group = state.group(),
          clientCert = state.serverCert(),
          serverCert = state.serverCert(),
          alpn = state.alpn(),
          handshakeTime = *state.handshakeTime(),
          clock = state.context()->getClock()](
             KeyScheduler& ks, NewSessionTicket nst) {
    CachedPsk psk;
    psk.type = PskType::Resumption;
    setPskAndSecret(ks, psk, nst, resumptionSecret->coalesce());
    psk.version = version;
    psk.cipher = cipher;
    psk.group = group;
    psk.serverCert = serverCert;
    psk.clientCert = clientCert;
    psk.alpn = alpn;
    psk.ticketAgeAdd = nst.ticket_age_add;
    psk.ticketIssueTime = clock->getCurrentTime();
    psk.ticketExpirationTime =
        clock->getCurrentTime() + std::chrono::seconds(nst.ticket_lifetime);
    psk.ticketHandshakeTime = handshakeTime;
    psk.maxEarlyDataSize = getMaxEarlyDataSize(nst);

    VLOG(10) << "kTLS callback processed NST. pskIdentity=" << pskIdentity;
    pskCache->putPsk(pskIdentity, std::move(psk));
  };
}
} // namespace detail
} // namespace fizz
