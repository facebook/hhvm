/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Utils.h>
#include <fizz/crypto/exchange/AsyncKeyExchange.h>
#include <fizz/protocol/CertificateVerifier.h>
#include <fizz/protocol/Protocol.h>
#include <fizz/protocol/StateMachine.h>
#include <fizz/protocol/ech/Decrypter.h>
#include <fizz/record/Extensions.h>
#include <fizz/record/PlaintextRecordLayer.h>
#include <fizz/server/AsyncSelfCert.h>
#include <fizz/server/Negotiator.h>
#include <fizz/server/ReplayCache.h>
#include <fizz/server/ServerProtocol.h>
#include <fizz/util/Workarounds.h>
#include <folly/Overload.h>
#include <folly/tracing/StaticTracepoint.h>
#include <algorithm>
#include <set>

using folly::Optional;
using folly::SemiFuture;

using namespace fizz::server;
using namespace fizz::server::detail;

// We only ever use the first PSK sent.
static constexpr uint16_t kPskIndex = 0;

namespace fizz {
namespace sm {

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::Uninitialized,
    Event::Accept,
    StateEnum::ExpectingClientHello);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::ExpectingClientHello,
    Event::ClientHello,
    StateEnum::ExpectingClientHello,
    StateEnum::ExpectingCertificate,
    StateEnum::ExpectingFinished,
    StateEnum::AcceptingEarlyData,
    StateEnum::Error);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::AcceptingEarlyData,
    Event::AppData,
    StateEnum::Error);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::AcceptingEarlyData,
    Event::AppWrite,
    StateEnum::Error);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::AcceptingEarlyData,
    Event::EndOfEarlyData,
    StateEnum::ExpectingFinished);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::ExpectingCertificate,
    Event::Certificate,
    StateEnum::ExpectingCertificateVerify,
    StateEnum::ExpectingFinished);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::ExpectingCertificateVerify,
    Event::CertificateVerify,
    StateEnum::ExpectingFinished);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::ExpectingFinished,
    Event::AppWrite,
    StateEnum::Error);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::ExpectingFinished,
    Event::Finished,
    StateEnum::AcceptingData);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::AcceptingData,
    Event::WriteNewSessionTicket,
    StateEnum::Error);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::AcceptingData,
    Event::AppData,
    StateEnum::Error);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::AcceptingData,
    Event::AppWrite,
    StateEnum::Error);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::AcceptingData,
    Event::KeyUpdateInitiation,
    StateEnum::AcceptingData);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::AcceptingData,
    Event::KeyUpdate,
    StateEnum::AcceptingData);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::AcceptingData,
    Event::CloseNotify,
    StateEnum::Closed);

FIZZ_DECLARE_EVENT_HANDLER(
    ServerTypes,
    StateEnum::ExpectingCloseNotify,
    Event::CloseNotify,
    StateEnum::Closed);
} // namespace sm

namespace server {

AsyncActions ServerStateMachine::processAccept(
    const State& state,
    folly::Executor* executor,
    std::shared_ptr<const FizzServerContext> context,
    const std::shared_ptr<ServerExtensions>& extensions) {
  Accept accept;
  accept.executor = executor;
  accept.context = std::move(context);
  accept.extensions = extensions;
  return detail::processEvent(state, std::move(accept));
}

AsyncActions ServerStateMachine::processSocketData(
    const State& state,
    folly::IOBufQueue& buf,
    Aead::AeadOptions options) {
  try {
    if (!state.readRecordLayer()) {
      return detail::handleError(
          state,
          ReportError("attempting to process data without record layer"),
          folly::none);
    }
    auto param = state.readRecordLayer()->readEvent(buf, std::move(options));
    if (!param.has_value()) {
      return actions(WaitForData{param.sizeHint});
    }
    return detail::processEvent(state, std::move(*param));
  } catch (const FizzException& e) {
    return detail::handleError(
        state,
        ReportError(folly::exception_wrapper(std::current_exception())),
        e.getAlert());
  } catch (...) {
    return detail::handleError(
        state,
        ReportError(folly::make_exception_wrapper<FizzException>(
            folly::to<std::string>(
                "error decoding record in state ",
                toString(state.state()),
                ": ",
                folly::exceptionStr(std::current_exception())),
            AlertDescription::decode_error)),
        AlertDescription::decode_error);
  }
}

AsyncActions ServerStateMachine::processWriteNewSessionTicket(
    const State& state,
    WriteNewSessionTicket write) {
  return detail::processEvent(state, std::move(write));
}

AsyncActions ServerStateMachine::processAppWrite(
    const State& state,
    AppWrite write) {
  return detail::processEvent(state, std::move(write));
}

AsyncActions ServerStateMachine::processEarlyAppWrite(
    const State& state,
    EarlyAppWrite write) {
  return detail::processEvent(state, std::move(write));
}

Actions ServerStateMachine::processAppClose(const State& state) {
  return detail::handleAppClose(state);
}

Actions ServerStateMachine::processAppCloseImmediate(const State& state) {
  return detail::handleAppCloseImmediate(state);
}

AsyncActions ServerStateMachine::processKeyUpdateInitiation(
    const State& state,
    KeyUpdateInitiation keyUpdateInitiation) {
  return detail::processEvent(state, std::move(keyUpdateInitiation));
}

namespace detail {

AsyncActions processEvent(const State& state, Param param) {
  auto event = EventVisitor()(param);
  // We can have an exception directly in the handler or in a future so we need
  // to handle both types.
  try {
    auto actions = sm::StateMachine<ServerTypes>::getHandler(
        state.state(), event)(state, std::move(param));

    return folly::variant_match(
        actions,
        ::fizz::detail::result_type<AsyncActions>(),
        [&state](SemiFuture<Actions>& futureActions) -> AsyncActions {
          if (futureActions.isReady()) {
            // any exception thrown by get will be caught below
            return std::move(futureActions).get();
          }
          return std::move(futureActions)
              .deferError([&state](folly::exception_wrapper ew) {
                auto ex = ew.get_exception<FizzException>();
                if (ex) {
                  return detail::handleError(
                      state, ReportError(std::move(ew)), ex->getAlert());
                }
                return detail::handleError(
                    state,
                    ReportError(std::move(ew)),
                    AlertDescription::unexpected_message);
              });
        },
        [](Actions& immediateActions) -> AsyncActions {
          return std::move(immediateActions);
        });
  } catch (const FizzException& e) {
    return detail::handleError(
        state,
        ReportError(folly::exception_wrapper(std::current_exception())),
        e.getAlert());
  } catch (...) {
    return detail::handleError(
        state,
        ReportError(folly::exception_wrapper(std::current_exception())),
        AlertDescription::unexpected_message);
  }
}

Actions handleError(
    const State& state,
    ReportError error,
    Optional<AlertDescription> alertDesc) {
  if (state.state() == StateEnum::Error) {
    return Actions();
  }
  MutateState transition([](State& newState) {
    newState.state() = StateEnum::Error;
    newState.writeRecordLayer() = nullptr;
    newState.readRecordLayer() = nullptr;
  });
  if (alertDesc && state.writeRecordLayer()) {
    Alert alert(*alertDesc);
    WriteToSocket write;
    write.contents.emplace_back(
        state.writeRecordLayer()->writeAlert(std::move(alert)));
    return actions(std::move(transition), std::move(write), std::move(error));
  } else {
    return actions(std::move(transition), std::move(error));
  }
}

Actions handleAppCloseImmediate(const State& state) {
  MutateState transition([](State& newState) {
    newState.state() = StateEnum::Closed;
    newState.readRecordLayer() = nullptr;
    newState.writeRecordLayer() = nullptr;
  });

  if (state.writeRecordLayer()) {
    Alert alert(AlertDescription::close_notify);
    WriteToSocket write;
    write.contents.emplace_back(
        state.writeRecordLayer()->writeAlert(std::move(alert)));
    return actions(std::move(transition), std::move(write));
  } else {
    return actions(std::move(transition));
  }
}

Actions handleAppClose(const State& state) {
  if (state.writeRecordLayer()) {
    MutateState transition([](State& newState) {
      newState.state() = StateEnum::ExpectingCloseNotify;
      newState.writeRecordLayer() = nullptr;
    });

    Alert alert(AlertDescription::close_notify);
    WriteToSocket write;
    write.contents.emplace_back(
        state.writeRecordLayer()->writeAlert(std::move(alert)));
    return actions(std::move(transition), std::move(write));
  } else {
    MutateState transition([](State& newState) {
      newState.state() = StateEnum::Closed;
      newState.writeRecordLayer() = nullptr;
      newState.readRecordLayer() = nullptr;
    });
    return actions(std::move(transition));
  }
}

Actions handleInvalidEvent(const State& state, Event event, Param param) {
  if (event == Event::Alert) {
    auto& alert = *param.asAlert();
    throw FizzException(
        folly::to<std::string>(
            "received alert: ",
            toString(alert.description),
            ", in state ",
            toString(state.state())),
        folly::none);
  } else {
    throw FizzException(
        folly::to<std::string>(
            "invalid event: ",
            toString(event),
            ", in state ",
            toString(state.state())),
        AlertDescription::unexpected_message);
  }
}

template <typename T, typename F>
SemiFuture<
    typename folly::futures::detail::valueCallableResult<T, F>::value_type>
runOnCallerIfComplete(
    folly::Executor* executor,
    SemiFuture<T> future,
    F&& func) {
  if (future.isReady()) {
    return func(std::move(future).get());
  } else {
    return std::move(future)
        .via(executor)
        .thenValueInline(std::forward<F>(func))
        .semi();
  }
}

} // namespace detail
} // namespace server

namespace sm {

static void ensureNoUnparsedHandshakeData(const State& state, Event event) {
  if (state.readRecordLayer()->hasUnparsedHandshakeData()) {
    throw FizzException(
        folly::to<std::string>(
            "unprocessed handshake data while handling event ",
            toString(event),
            " in state ",
            toString(state.state())),
        AlertDescription::unexpected_message);
  }
}

AsyncActions
EventHandler<ServerTypes, StateEnum::Uninitialized, Event::Accept>::handle(
    const State& /*state*/,
    Param param) {
  auto& accept = *param.asAccept();
  auto factory = accept.context->getFactory();
  auto readRecordLayer = factory->makePlaintextReadRecordLayer();
  auto writeRecordLayer = factory->makePlaintextWriteRecordLayer();
  auto handshakeLogging = std::make_unique<HandshakeLogging>();
  return actions(
      MutateState([executor = accept.executor,
                   rrl = std::move(readRecordLayer),
                   wrl = std::move(writeRecordLayer),
                   context = std::move(accept.context),
                   handshakeLogging = std::move(handshakeLogging),
                   extensions = accept.extensions](State& newState) mutable {
        newState.executor() = executor;
        newState.context() = std::move(context);
        newState.readRecordLayer() = std::move(rrl);
        newState.writeRecordLayer() = std::move(wrl);
        newState.handshakeLogging() = std::move(handshakeLogging);
        newState.extensions() = std::move(extensions);
      }),
      MutateState(&Transition<StateEnum::ExpectingClientHello>));
}

static void addHandshakeLogging(const State& state, const ClientHello& chlo) {
  auto logging = state.handshakeLogging();
  if (!logging) {
    return;
  }
  logging->populateFromClientHello(chlo);
  auto plaintextReadRecord =
      dynamic_cast<PlaintextReadRecordLayer*>(state.readRecordLayer());
  if (plaintextReadRecord) {
    logging->clientRecordVersion =
        plaintextReadRecord->getReceivedRecordVersion();
  }
}

static void validateClientHello(const ClientHello& chlo) {
  if (chlo.legacy_compression_methods.size() != 1 ||
      chlo.legacy_compression_methods.front() != 0x00) {
    throw FizzException(
        "client compression methods not exactly NULL",
        AlertDescription::illegal_parameter);
  }
  Protocol::checkDuplicateExtensions(chlo.extensions);
}

static Optional<ProtocolVersion> negotiateVersion(
    const ClientHello& chlo,
    const std::vector<ProtocolVersion>& versions) {
  const auto& clientVersions = getExtension<SupportedVersions>(chlo.extensions);
  if (!clientVersions) {
    return folly::none;
  }
  auto version = negotiate(versions, clientVersions->versions);
  if (!version) {
    return folly::none;
  }
  return version;
}

static Optional<CookieState> getCookieState(
    const ClientHello& chlo,
    const CookieCipher* cookieCipher) {
  auto cookieExt = getExtension<Cookie>(chlo.extensions);
  if (!cookieExt) {
    return folly::none;
  }

  // If the client sent a cookie we can't use we have to consider it a fatal
  // error since we can't reconstruct the handshake transcript.
  if (!cookieCipher) {
    throw FizzException(
        "no cookie cipher", AlertDescription::unsupported_extension);
  }

  auto cookieState = cookieCipher->decrypt(std::move(cookieExt->cookie));

  if (!cookieState) {
    throw FizzException(
        "could not decrypt cookie", AlertDescription::decrypt_error);
  }

  if (cookieState->echCipherSuite.has_value() ^
      cookieState->echConfigId.has_value()) {
    throw FizzException(
        "cookie has incomplete ech params", AlertDescription::internal_error);
  }

  return cookieState;
}

static void verifyCookieState(
    Optional<CookieState>& cookieState,
    ProtocolVersion version,
    CipherSuite cipher) {
  if (!cookieState) {
    return;
  }
  if (cookieState->version != version) {
    throw FizzException(
        "version mismatch with cookie", AlertDescription::protocol_version);
  }

  if (cookieState->cipher != cipher) {
    throw FizzException(
        "cipher mismatch with cookie", AlertDescription::handshake_failure);
  }
}

namespace {
struct ResumptionStateResult {
  explicit ResumptionStateResult(
      SemiFuture<std::pair<PskType, Optional<ResumptionState>>>
          futureResStateArg,
      Optional<PskKeyExchangeMode> pskModeArg = folly::none,
      Optional<uint32_t> obfuscatedAgeArg = folly::none)
      : futureResState(std::move(futureResStateArg)),
        pskMode(std::move(pskModeArg)),
        obfuscatedAge(std::move(obfuscatedAgeArg)) {}

  SemiFuture<std::pair<PskType, Optional<ResumptionState>>> futureResState;
  Optional<PskKeyExchangeMode> pskMode;
  Optional<uint32_t> obfuscatedAge;
};
} // namespace

static ResumptionStateResult getResumptionState(
    const ClientHello& chlo,
    const TicketCipher* ticketCipher,
    const std::vector<PskKeyExchangeMode>& supportedModes) {
  auto psks = getExtension<ClientPresharedKey>(chlo.extensions);
  auto clientModes = getExtension<PskKeyExchangeModes>(chlo.extensions);
  if (psks && !clientModes) {
    throw FizzException("no psk modes", AlertDescription::missing_extension);
  }

  Optional<PskKeyExchangeMode> pskMode;
  if (clientModes) {
    pskMode = negotiate(supportedModes, clientModes->modes);
  }
  if (!psks && !pskMode) {
    FOLLY_SDT(fizz, session_cache_NotSupported);
    return ResumptionStateResult(
        std::make_pair(PskType::NotSupported, folly::none));
  } else if (!psks || psks->identities.size() <= kPskIndex) {
    FOLLY_SDT(fizz, session_cache_NotAttempted);
    return ResumptionStateResult(
        std::make_pair(PskType::NotAttempted, folly::none));
  } else if (!ticketCipher) {
    FOLLY_SDT(fizz, session_cache_NoTicketCipher);
    VLOG(8) << "No ticket cipher, rejecting PSK.";
    return ResumptionStateResult(
        std::make_pair(PskType::Rejected, folly::none));
  } else if (!pskMode) {
    FOLLY_SDT(fizz, session_cache_PskModeMismatch);
    VLOG(8) << "No psk mode match, rejecting PSK.";
    return ResumptionStateResult(
        std::make_pair(PskType::Rejected, folly::none));
  } else {
    FOLLY_SDT(fizz, session_cache_ResumptionSuccess);
    const auto& ident = psks->identities[kPskIndex].psk_identity;
    return ResumptionStateResult(
        ticketCipher->decrypt(ident->clone()),
        pskMode,
        psks->identities[kPskIndex].obfuscated_ticket_age);
  }
}

static SemiFuture<ReplayCacheResult> getReplayCacheResult(
    const ClientHello& chlo,
    bool zeroRttEnabled,
    ReplayCache* replayCache) {
  if (!zeroRttEnabled || !replayCache ||
      !getExtension<ClientEarlyData>(chlo.extensions)) {
    FOLLY_SDT(fizz, replay_cache_NotChecked);
    return ReplayCacheResult::NotChecked;
  }
  auto randBuf = folly::IOBuf::copyBuffer(chlo.random, chlo.random.size());
  return replayCache->check(std::move(randBuf));
}

static bool validateResumptionState(
    const ResumptionState& resState,
    PskKeyExchangeMode /* mode */,
    ProtocolVersion version,
    CipherSuite cipher) {
  if (resState.version != version) {
    FOLLY_SDT(fizz, resumption_state_ProtocolVersionMismatch);
    VLOG(8) << "Protocol version mismatch, rejecting PSK.";
    return false;
  }

  if (getHashFunction(resState.cipher) != getHashFunction(cipher)) {
    FOLLY_SDT(fizz, resumption_state_HashFunctionMismatch);
    VLOG(8) << "Hash mismatch, rejecting PSK.";
    return false;
  }

  return true;
}

static CipherSuite negotiateCipher(
    const ClientHello& chlo,
    const std::vector<std::vector<CipherSuite>>& supportedCiphers) {
  auto cipher = negotiate(supportedCiphers, chlo.cipher_suites);
  if (!cipher) {
    throw FizzException("no cipher match", AlertDescription::handshake_failure);
  }
  return *cipher;
}

/*
 * Sets up a KeyScheduler and HandshakeContext for the connection. The
 * KeyScheduler will have the early secret derived if applicable, and the
 * ClientHello will be added to the HandshakeContext. This also verifies the
 * PSK binder if applicable.
 *
 * If the passed in handshakeContext is non-null it is used instead of a new
 * context. This is used after a HelloRetryRequest when there is already a
 * handshake transcript before the current ClientHello.
 */
static std::
    pair<std::unique_ptr<KeyScheduler>, std::unique_ptr<HandshakeContext>>
    setupSchedulerAndContext(
        const Factory& factory,
        CipherSuite cipher,
        const ClientHello& chlo,
        const Optional<ResumptionState>& resState,
        const Optional<CookieState>& cookieState,
        PskType pskType,
        std::unique_ptr<HandshakeContext> handshakeContext,
        ProtocolVersion /*version*/) {
  auto scheduler = factory.makeKeyScheduler(cipher);

  if (cookieState) {
    handshakeContext = factory.makeHandshakeContext(cipher);

    message_hash chloHash;
    chloHash.hash = cookieState->chloHash->clone();
    handshakeContext->appendToTranscript(encodeHandshake(std::move(chloHash)));

    auto cookie = getExtension<Cookie>(chlo.extensions);
    handshakeContext->appendToTranscript(getStatelessHelloRetryRequest(
        cookieState->version,
        cookieState->cipher,
        cookieState->group,
        std::move(cookie->cookie)));
  } else if (!handshakeContext) {
    handshakeContext = factory.makeHandshakeContext(cipher);
  }

  if (resState) {
    scheduler->deriveEarlySecret(resState->resumptionSecret->coalesce());

    auto binderKey = scheduler
                         ->getSecret(
                             pskType == PskType::External
                                 ? EarlySecrets::ExternalPskBinder
                                 : EarlySecrets::ResumptionPskBinder,
                             handshakeContext->getBlankContext())
                         .secret;

    folly::IOBufQueue chloQueue(folly::IOBufQueue::cacheChainLength());
    chloQueue.append((*chlo.originalEncoding)->clone());
    auto chloPrefix =
        chloQueue.split(chloQueue.chainLength() - getBinderLength(chlo));
    handshakeContext->appendToTranscript(chloPrefix);

    const auto& psks = getExtension<ClientPresharedKey>(chlo.extensions);
    if (!psks || psks->binders.size() <= kPskIndex) {
      throw FizzException("no binders", AlertDescription::illegal_parameter);
    }
    auto expectedBinder =
        handshakeContext->getFinishedData(folly::range(binderKey));
    if (!CryptoUtils::equal(
            expectedBinder->coalesce(),
            psks->binders[kPskIndex].binder->coalesce())) {
      throw FizzException(
          "binder does not match", AlertDescription::bad_record_mac);
    }

    handshakeContext->appendToTranscript(chloQueue.move());
    return std::make_pair(std::move(scheduler), std::move(handshakeContext));
  } else {
    handshakeContext->appendToTranscript(*chlo.originalEncoding);
    return std::make_pair(std::move(scheduler), std::move(handshakeContext));
  }
}

static void validateGroups(const std::vector<KeyShareEntry>& client_shares) {
  std::set<NamedGroup> setOfNamedGroups;

  for (const auto& share : client_shares) {
    if (setOfNamedGroups.find(share.group) != setOfNamedGroups.end()) {
      throw FizzException(
          "duplicate client key share", AlertDescription::illegal_parameter);
    }

    setOfNamedGroups.insert(share.group);
  }
}

static std::tuple<NamedGroup, Optional<Buf>> negotiateGroup(
    ProtocolVersion /*version*/,
    const ClientHello& chlo,
    const std::vector<NamedGroup>& supportedGroups) {
  auto groups = getExtension<SupportedGroups>(chlo.extensions);
  if (!groups) {
    throw FizzException("no named groups", AlertDescription::missing_extension);
  }
  auto group = negotiate(supportedGroups, groups->named_group_list);
  if (!group) {
    throw FizzException("no group match", AlertDescription::handshake_failure);
  }
  auto clientShares = getExtension<ClientKeyShare>(chlo.extensions);
  if (!clientShares) {
    throw FizzException(
        "no client shares", AlertDescription::missing_extension);
  }

  validateGroups(clientShares->client_shares);
  for (const auto& share : clientShares->client_shares) {
    if (share.group == *group) {
      return std::make_tuple(*group, share.key_exchange->clone());
    }
  }
  return std::make_tuple(*group, folly::none);
}

static SemiFuture<Optional<AsyncKeyExchange::DoKexResult>> doKexSyncFuture(
    KeyExchange* kex,
    std::unique_ptr<folly::IOBuf> clientShare) {
  AsyncKeyExchange::DoKexResult res;
  kex->generateKeyPair();
  res.sharedSecret = kex->generateSharedSecret(clientShare->coalesce());
  res.ourKeyShare = kex->getKeyShare();
  return folly::makeSemiFuture(Optional(std::move(res)));
  // Everything completed and clientShare can be safely freed now.
}

// Caller is responsible to hold pKex until the lambda finished.
static SemiFuture<Optional<AsyncKeyExchange::DoKexResult>> doKexFuture(
    KeyExchange* pKex,
    std::unique_ptr<folly::IOBuf> clientShare) {
  auto pAsyncKex = dynamic_cast<AsyncKeyExchange*>(pKex);
  if (pAsyncKex) {
    auto fut = pAsyncKex->doAsyncKexFuture(std::move(clientShare));
    return std::move(fut).deferValue([](AsyncKeyExchange::DoKexResult res) {
      return Optional(std::move(res));
    });
  } else {
    return doKexSyncFuture(pKex, std::move(clientShare));
  }
}

static HelloRetryRequest getHelloRetryRequest(
    ProtocolVersion version,
    CipherSuite cipher,
    NamedGroup group,
    Buf legacySessionId,
    HandshakeContext& /*handshakeContext*/) {
  HelloRetryRequest hrr;
  hrr.legacy_version = ProtocolVersion::tls_1_2;
  hrr.legacy_session_id_echo = std::move(legacySessionId);
  hrr.cipher_suite = cipher;
  ServerSupportedVersions versionExt;
  versionExt.selected_version = version;
  hrr.extensions.push_back(encodeExtension(std::move(versionExt)));
  HelloRetryRequestKeyShare keyShare;
  keyShare.selected_group = group;
  hrr.extensions.push_back(encodeExtension(std::move(keyShare)));
  return hrr;
}

static ServerHello getServerHello(
    ProtocolVersion version,
    Random random,
    CipherSuite cipher,
    bool psk,
    Optional<NamedGroup> group,
    Optional<Buf> serverShare,
    Buf legacySessionId) {
  ServerHello serverHello;

  serverHello.legacy_version = ProtocolVersion::tls_1_2;
  ServerSupportedVersions versionExt;
  versionExt.selected_version = version;
  serverHello.extensions.push_back(encodeExtension(std::move(versionExt)));
  serverHello.legacy_session_id_echo = std::move(legacySessionId);

  serverHello.random = std::move(random);
  serverHello.cipher_suite = cipher;
  if (group) {
    ServerKeyShare serverKeyShare;
    serverKeyShare.server_share.group = *group;
    serverKeyShare.server_share.key_exchange = std::move(*serverShare);

    serverHello.extensions.push_back(
        encodeExtension(std::move(serverKeyShare)));
  }
  if (psk) {
    ServerPresharedKey serverPsk;
    serverPsk.selected_identity = kPskIndex;
    serverHello.extensions.push_back(encodeExtension(std::move(serverPsk)));
  }
  return serverHello;
}

static Optional<std::string> negotiateAlpn(
    const ClientHello& chlo,
    folly::Optional<std::string> zeroRttAlpn,
    const FizzServerContext& context) {
  auto ext = getExtension<ProtocolNameList>(chlo.extensions);
  std::vector<std::string> clientProtocols;
  // Check whether client supports ALPN
  if (ext) {
    for (auto& protocol : ext->protocol_name_list) {
      clientProtocols.push_back(protocol.name->moveToFbString().toStdString());
    }
  } else {
    VLOG(6) << "Client did not send ALPN extension";
    if (context.getAlpnMode() == AlpnMode::Required) {
      throw FizzException(
          "ALPN is required", AlertDescription::no_application_protocol);
    }
    return folly::none;
  }

  // Since both support ALPN, check whether the protocols match.
  // Server's support for ALPN is to be enforced at the configuration.
  auto selected = context.negotiateAlpn(clientProtocols, zeroRttAlpn);
  if (!selected) {
    VLOG(6) << "ALPN mismatch";
    if (context.getAlpnMode() != AlpnMode::AllowMismatch) {
      auto msg = context.getAlpnMode() == AlpnMode::Optional
          ? "Unable to negotiate ALPN, as required by policy. policy=AlpnMode::Optional"
          : "Unable to negotiate ALPN, as required by policy. policy=AlpnMode::Required";
      throw FizzException(msg, AlertDescription::no_application_protocol);
    }
  } else {
    VLOG(6) << "ALPN: " << *selected;
  }
  return selected;
}

static Optional<std::chrono::milliseconds> getClockSkew(
    const Optional<ResumptionState>& psk,
    Optional<uint32_t> obfuscatedAge,
    const std::chrono::system_clock::time_point& currentTime) {
  if (!psk || !obfuscatedAge) {
    return folly::none;
  }

  auto age = std::chrono::milliseconds(
      static_cast<uint32_t>(*obfuscatedAge - psk->ticketAgeAdd));

  auto expected = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - psk->ticketIssueTime);

  return std::chrono::milliseconds(age - expected);
}

static Optional<Buf> getAppToken(const Optional<ResumptionState>& psk) {
  if (!psk.has_value() || !psk->appToken) {
    return folly::none;
  }
  return psk->appToken->clone();
}

static EarlyDataType negotiateEarlyDataType(
    bool acceptEarlyData,
    const ClientHello& chlo,
    const Optional<ResumptionState>& psk,
    CipherSuite cipher,
    Optional<KeyExchangeType> keyExchangeType,
    const Optional<CookieState>& cookieState,
    Optional<std::string> alpn,
    ReplayCacheResult replayCacheResult,
    Optional<std::chrono::milliseconds> clockSkew,
    ClockSkewTolerance clockSkewTolerance,
    const AppTokenValidator* appTokenValidator) {
  if (!getExtension<ClientEarlyData>(chlo.extensions)) {
    return EarlyDataType::NotAttempted;
  }

  if (!acceptEarlyData) {
    VLOG(5) << "Rejecting early data: disabled";
    return EarlyDataType::Rejected;
  }

  if (!psk) {
    VLOG(5) << "Rejected early data: psk rejected";
    return EarlyDataType::Rejected;
  }

  if (psk->cipher != cipher) {
    VLOG(5) << "Rejected early data: cipher mismatch";
    return EarlyDataType::Rejected;
  }

  if (psk->alpn != alpn) {
    VLOG(5) << "Rejecting early data: alpn mismatch";
    return EarlyDataType::Rejected;
  }

  if (keyExchangeType &&
      *keyExchangeType == KeyExchangeType::HelloRetryRequest) {
    VLOG(5) << "Rejecting early data: HelloRetryRequest";
    return EarlyDataType::Rejected;
  }

  if (cookieState) {
    VLOG(5) << "Rejecting early data: Cookie";
    return EarlyDataType::Rejected;
  }

  if (replayCacheResult != ReplayCacheResult::NotReplay) {
    VLOG(5) << "Rejecting early data: replay";
    return EarlyDataType::Rejected;
  }

  if (!clockSkew || *clockSkew < clockSkewTolerance.before ||
      *clockSkew > clockSkewTolerance.after) {
    VLOG(5) << "Rejecting early data: clock skew clockSkew="
            << (clockSkew ? folly::to<std::string>(clockSkew->count())
                          : "(none)")
            << " toleranceBefore=" << clockSkewTolerance.before.count()
            << " toleranceAfter=" << clockSkewTolerance.after.count();
    return EarlyDataType::Rejected;
  }

  if (appTokenValidator && !appTokenValidator->validate(*psk)) {
    VLOG(5) << "Rejecting early data: invalid app token";
    return EarlyDataType::Rejected;
  }

  return EarlyDataType::Accepted;
}

static Buf getEncryptedExt(
    HandshakeContext& handshakeContext,
    const folly::Optional<std::string>& selectedAlpn,
    EarlyDataType earlyData,
    folly::Optional<std::vector<ech::ECHConfig>> echRetryConfigs,
    std::vector<Extension> otherExtensions) {
  EncryptedExtensions encryptedExt;
  if (selectedAlpn) {
    ProtocolNameList alpn;
    ProtocolName protocol;
    protocol.name = folly::IOBuf::copyBuffer(*selectedAlpn);
    alpn.protocol_name_list.push_back(std::move(protocol));
    encryptedExt.extensions.push_back(encodeExtension(std::move(alpn)));
  }

  if (earlyData == EarlyDataType::Accepted) {
    encryptedExt.extensions.push_back(encodeExtension(ServerEarlyData()));
  }

  if (echRetryConfigs.has_value()) {
    ech::ECHEncryptedExtensions serverEch;
    serverEch.retry_configs = std::move(*echRetryConfigs);
    encryptedExt.extensions.push_back(encodeExtension(std::move(serverEch)));
  }

  for (auto& ext : otherExtensions) {
    encryptedExt.extensions.push_back(std::move(ext));
  }
  auto encodedEncryptedExt =
      encodeHandshake<EncryptedExtensions>(std::move(encryptedExt));
  handshakeContext.appendToTranscript(encodedEncryptedExt);
  return encodedEncryptedExt;
}

static std::pair<std::shared_ptr<SelfCert>, SignatureScheme> chooseCert(
    const FizzServerContext& context,
    const ClientHello& chlo) {
  const auto& clientSigSchemes =
      getExtension<SignatureAlgorithms>(chlo.extensions);
  if (!clientSigSchemes) {
    throw FizzException("no sig schemes", AlertDescription::missing_extension);
  }
  Optional<std::string> sni;
  auto serverNameList = getExtension<ServerNameList>(chlo.extensions);
  if (serverNameList && !serverNameList->server_name_list.empty()) {
    sni = serverNameList->server_name_list.front()
              .hostname->moveToFbString()
              .toStdString();
  }

  auto certAndScheme = context.getCert(
      sni, clientSigSchemes->supported_signature_algorithms, chlo.extensions);
  if (!certAndScheme) {
    throw FizzException(
        "could not find suitable cert", AlertDescription::handshake_failure);
  }
  return *certAndScheme;
}

static std::tuple<Buf, folly::Optional<CertificateCompressionAlgorithm>>
getCertificate(
    const std::shared_ptr<const SelfCert>& serverCert,
    const FizzServerContext& context,
    const ClientHello& chlo,
    HandshakeContext& handshakeContext) {
  // Check for compression support first, and if so, send compressed.
  Buf encodedCertificate;
  folly::Optional<CertificateCompressionAlgorithm> algo;
  auto compAlgos =
      getExtension<CertificateCompressionAlgorithms>(chlo.extensions);
  if (compAlgos && !context.getSupportedCompressionAlgorithms().empty()) {
    algo = negotiate(
        context.getSupportedCompressionAlgorithms(), compAlgos->algorithms);
  }

  if (algo) {
    encodedCertificate = encodeHandshake(serverCert->getCompressedCert(*algo));
  } else {
    encodedCertificate = encodeHandshake(serverCert->getCertMessage());
  }
  handshakeContext.appendToTranscript(encodedCertificate);
  return std::make_tuple(std::move(encodedCertificate), std::move(algo));
}

static Buf getCertificateVerify(
    SignatureScheme sigScheme,
    Buf signature,
    HandshakeContext& handshakeContext) {
  CertificateVerify verify;
  verify.algorithm = sigScheme;
  verify.signature = std::move(signature);
  auto encodedCertificateVerify = encodeHandshake(std::move(verify));
  handshakeContext.appendToTranscript(encodedCertificateVerify);
  return encodedCertificateVerify;
}

static Buf getCertificateRequest(
    const std::vector<SignatureScheme>& acceptableSigSchemes,
    const CertificateVerifier* const verifier,
    HandshakeContext& handshakeContext) {
  CertificateRequest request;
  SignatureAlgorithms algos;
  algos.supported_signature_algorithms = acceptableSigSchemes;
  request.extensions.push_back(encodeExtension(std::move(algos)));
  if (verifier) {
    auto verifierExtensions = verifier->getCertificateRequestExtensions();
    for (auto& ext : verifierExtensions) {
      request.extensions.push_back(std::move(ext));
    }
  }
  auto encodedCertificateRequest = encodeHandshake(std::move(request));
  handshakeContext.appendToTranscript(encodedCertificateRequest);
  return encodedCertificateRequest;
}

static std::string getSNI(const ClientHello& chlo) {
  auto serverNameList = getExtension<ServerNameList>(chlo.extensions);
  std::string sni;
  if (serverNameList && !serverNameList->server_name_list.empty()) {
    sni = serverNameList->server_name_list.front()
              .hostname->moveToFbString()
              .toStdString();
  }
  return sni;
}

static std::tuple<ECHStatus, uint8_t> processECHHRR(
    const Optional<CookieState>& cookieState,
    const State& state,
    ClientHello& chlo) {
  auto decrypter = state.context()->getECHDecrypter();
  auto echExt = getExtension<ech::OuterECHClientHello>(chlo.extensions);
  ECHStatus echStatus = state.echStatus();

  // Check for cookie ECH
  bool cookieHasECH = cookieState && cookieState->echCipherSuite.has_value();

  // Do checks for both cases.
  if (echStatus == ECHStatus::Accepted || cookieHasECH) {
    if (!decrypter) {
      throw FizzException(
          "ech accepted but no decrypter set up",
          AlertDescription::internal_error);
    }
    if (!echExt) {
      throw FizzException(
          "ech not sent for hrr", AlertDescription::missing_extension);
    }
    if (!echExt->enc->empty()) {
      throw FizzException(
          "hrr ech enc not empty", AlertDescription::illegal_parameter);
    }
  }

  if (echStatus == ECHStatus::Accepted) {
    // Stateful HRR first.
    if (state.echState()->cipherSuite != echExt->cipher_suite) {
      throw FizzException(
          "ech hrr cipher suite mismatch", AlertDescription::illegal_parameter);
    }
    if (state.echState()->configId != echExt->config_id) {
      throw FizzException(
          "ech hrr config id mismatch", AlertDescription::illegal_parameter);
    }

    chlo =
        decrypter->decryptClientHelloHRR(chlo, state.echState()->hpkeContext);

    return {ECHStatus::Accepted, echExt->config_id};
  } else if (cookieHasECH) {
    // Stateless HRR now
    if (*cookieState->echCipherSuite != echExt->cipher_suite) {
      throw FizzException(
          "ech hrr cipher suite mismatch", AlertDescription::illegal_parameter);
    }
    if (*cookieState->echConfigId != echExt->config_id) {
      throw FizzException(
          "ech hrr config id mismatch", AlertDescription::illegal_parameter);
    }

    chlo = decrypter->decryptClientHelloHRR(chlo, cookieState->echEnc);

    return {ECHStatus::Accepted, echExt->config_id};
  }

  // Just return the ECH status as is
  return {echStatus, 0};
}

/*
 * `processECH` looks for any encrypted_client_hello extension sent
 * by the client and attempts to decrypt it.
 *
 * @param cookieState  If set, the decoded cookie from the client containing
 previous state (for HRR).
 * @param state Fizz server state, containing an optionally configured ECH
 decrypter
 * @param chlo  ClientHello received by the client (the "outer" client hello).
 If ECH is successfully decrypted, after this function returns `chlo` will now
 contain the decrypted "inner" client hello.
 *
 * @return A pair with the following semantics:
 *             1. The `ECHStatus`, indicating whether ECH was successfully
 decoded.
 *             2.
 *               a. If unset, this implies that no encrypted_client_hello
 extension was sent by the client
 *               b. If set, this implies that a client has sent a
 encrypted_client_hello extension. The `ECHState` contains fields that are
 logged from the extension that the client sent and it also contains any
 auxiliary ECH-relevant informatio.
 */
static std::pair<ECHStatus, folly::Optional<ECHState>> processECH(
    const Optional<CookieState>& cookieState,
    const State& state,
    ClientHello& chlo) {
  // First, fetch current state (if any).
  folly::Optional<ECHState> echState;
  ECHStatus echStatus = state.echStatus();
  auto decrypter = state.context()->getECHDecrypter();

  if (state.handshakeContext() || cookieState) {
    echState = ECHState{};
    // Process ECH for HRR (if any)
    std::tie(echStatus, echState->configId) =
        processECHHRR(cookieState, state, chlo);
    if (state.echState().has_value()) {
      echState->hpkeContext = std::move(state.echState()->hpkeContext);
      echState->cipherSuite = state.echState()->cipherSuite;
    }
  } else {
    bool requestedECH =
        findExtension(chlo.extensions, ExtensionType::encrypted_client_hello) !=
        chlo.extensions.end();

    // ECHState is populated even if we do not have a valid decrypter
    // to aid in logging (and detecting misconfigurations)
    if (requestedECH) {
      auto echExt = getExtension<ech::OuterECHClientHello>(chlo.extensions);
      echState = ECHState{
          echExt->cipher_suite, echExt->config_id, nullptr, folly::none};
      if (decrypter) {
        auto gotChlo = decrypter->decryptClientHello(chlo);
        if (gotChlo.has_value()) {
          auto outerSni = getSNI(chlo);
          echStatus = ECHStatus::Accepted;
          echState->hpkeContext = std::move(gotChlo->context);
          echState->outerSni = outerSni;
          chlo = std::move(gotChlo->chlo);
        } else {
          echStatus = ECHStatus::Rejected;
        }
      }
    }
  }

  if (echStatus == ECHStatus::Accepted &&
      getExtension<ech::InnerECHClientHello>(chlo.extensions) == folly::none) {
    throw FizzException(
        "inner clienthello missing encrypted_client_hello",
        AlertDescription::missing_extension);
  }

  return std::make_pair(echStatus, std::move(echState));
}

AsyncActions
EventHandler<ServerTypes, StateEnum::ExpectingClientHello, Event::ClientHello>::
    handle(const State& state, Param param) {
  ClientHello chlo = std::move(*param.asClientHello());

  auto cookieState = getCookieState(chlo, state.context()->getCookieCipher());

  if (state.handshakeContext() && cookieState.has_value()) {
    throw FizzException(
        "cookie after statefull hrr", AlertDescription::illegal_parameter);
  }

  ECHStatus echStatus;
  folly::Optional<ECHState> echState;

  std::tie(echStatus, echState) = processECH(cookieState, state, chlo);

  addHandshakeLogging(state, chlo);

  if (state.readRecordLayer()->hasUnparsedHandshakeData()) {
    throw FizzException(
        "data after client hello", AlertDescription::unexpected_message);
  }

  auto version =
      negotiateVersion(chlo, state.context()->getSupportedVersions());

  if (state.version().has_value() &&
      (!version || *version != *state.version())) {
    throw FizzException(
        "version mismatch with previous negotiation",
        AlertDescription::illegal_parameter);
  }

  if (!version) {
    if (getExtension<ClientEarlyData>(chlo.extensions)) {
      throw FizzException(
          "supported version mismatch with early data",
          AlertDescription::protocol_version);
    }
    if (state.context()->getVersionFallbackEnabled()) {
      AttemptVersionFallback fallback;
      // Re-encode to put the record layer header back on. This won't
      // necessarily preserve it byte-for-byte, but it isn't authenticated so
      // should be ok.
      fallback.clientHello =
          PlaintextWriteRecordLayer()
              .writeInitialClientHello(std::move(*chlo.originalEncoding))
              .data;
      // Save SNI extension value to help decide server SSL context.
      auto serverNameList = getExtension<ServerNameList>(chlo.extensions);
      if (serverNameList && !serverNameList->server_name_list.empty()) {
        fallback.sni = serverNameList->server_name_list.front()
                           .hostname->moveToFbString()
                           .toStdString();
      }

      return actions(
          MutateState(&Transition<StateEnum::Error>), std::move(fallback));
    } else {
      throw FizzException(
          "supported version mismatch", AlertDescription::protocol_version);
    }
  }

  state.writeRecordLayer()->setProtocolVersion(*version);

  validateClientHello(chlo);

  auto cipher = negotiateCipher(chlo, state.context()->getSupportedCiphers());

  if (state.cipher().has_value() && cipher != *state.cipher()) {
    throw FizzException(
        "cipher mismatch with previous negotiation",
        AlertDescription::illegal_parameter);
  }

  verifyCookieState(cookieState, *version, cipher);

  auto resStateResult = getResumptionState(
      chlo,
      state.context()->getTicketCipher(),
      state.context()->getSupportedPskModes());

  auto replayCacheResultFuture = getReplayCacheResult(
      chlo,
      state.context()->getAcceptEarlyData(*version),
      state.context()->getReplayCache());

  auto results =
      collectAll(resStateResult.futureResState, replayCacheResultFuture);

  using FutureResultType = std::tuple<
      folly::Try<std::pair<PskType, Optional<ResumptionState>>>,
      folly::Try<ReplayCacheResult>>;
  return runOnCallerIfComplete(
      state.executor(),
      std::move(results),
      [&state,
       chlo = std::move(chlo),
       cookieState = std::move(cookieState),
       version = *version,
       cipher,
       pskMode = resStateResult.pskMode,
       echStatus,
       echState = std::move(echState),
       obfuscatedAge =
           resStateResult.obfuscatedAge](FutureResultType result) mutable {
        auto& resumption = *std::get<0>(result);
        auto pskType = resumption.first;
        auto resState = std::move(resumption.second);
        auto replayCacheResult = *std::get<1>(result);

        if (resState) {
          if (!validateResumptionState(*resState, *pskMode, version, cipher)) {
            pskType = PskType::Rejected;
            pskMode = folly::none;
            resState = folly::none;
          }
        } else {
          pskMode = folly::none;
        }

        auto legacySessionId = chlo.legacy_session_id->clone();

        // If we successfully resumed, set the handshake time to the ticket's
        // handshake time to preserve it across ticket updates. If not, set it
        // to now.
        std::chrono::system_clock::time_point handshakeTime;
        if (resState) {
          handshakeTime = resState->handshakeTime;
        } else {
          handshakeTime = state.context()->getClock().getCurrentTime();
        }

        std::unique_ptr<KeyScheduler> scheduler;
        std::unique_ptr<HandshakeContext> handshakeContext;
        std::tie(scheduler, handshakeContext) = setupSchedulerAndContext(
            *state.context()->getFactory(),
            cipher,
            chlo,
            resState,
            cookieState,
            pskType,
            std::move(state.handshakeContext()),
            version);

        auto alpn = negotiateAlpn(chlo, folly::none, *state.context());

        auto clockSkew = getClockSkew(
            resState,
            obfuscatedAge,
            state.context()->getClock().getCurrentTime());

        auto appToken = getAppToken(resState);

        auto earlyDataType = negotiateEarlyDataType(
            state.context()->getAcceptEarlyData(version),
            chlo,
            resState,
            cipher,
            state.keyExchangeType(),
            cookieState,
            alpn,
            replayCacheResult,
            clockSkew,
            state.context()->getClockSkewTolerance(),
            state.appTokenValidator());

        std::unique_ptr<EncryptedReadRecordLayer> earlyReadRecordLayer;
        Buf earlyExporterMaster;
        folly::Optional<SecretAvailable> earlyReadSecretAvailable;
        if (earlyDataType == EarlyDataType::Accepted) {
          auto earlyContext = handshakeContext->getHandshakeContext();
          auto earlyReadSecret = scheduler->getSecret(
              EarlySecrets::ClientEarlyTraffic, earlyContext->coalesce());
          if (!state.context()->getOmitEarlyRecordLayer()) {
            earlyReadRecordLayer =
                state.context()->getFactory()->makeEncryptedReadRecordLayer(
                    EncryptionLevel::EarlyData);
            earlyReadRecordLayer->setProtocolVersion(version);

            Protocol::setAead(
                *earlyReadRecordLayer,
                cipher,
                folly::range(earlyReadSecret.secret),
                *state.context()->getFactory(),
                *scheduler);
          }

          earlyReadSecretAvailable =
              SecretAvailable(std::move(earlyReadSecret));
          earlyExporterMaster = folly::IOBuf::copyBuffer(
              scheduler
                  ->getSecret(
                      EarlySecrets::EarlyExporter, earlyContext->coalesce())
                  .secret);
        }

        Optional<NamedGroup> group;
        KeyExchangeType keyExchangeType;
        SemiFuture<Optional<AsyncKeyExchange::DoKexResult>> kexResultFuture =
            folly::none;
        std::unique_ptr<KeyExchange> kex = nullptr;

        if (!pskMode || *pskMode != PskKeyExchangeMode::psk_ke) {
          Optional<Buf> clientShare;
          std::tie(group, clientShare) = negotiateGroup(
              version, chlo, state.context()->getSupportedGroups());
          if (!clientShare) {
            VLOG(8) << "Did not find key share for " << toString(*group);
            if (state.group().has_value() || cookieState) {
              throw FizzException(
                  "key share not found for already negotiated group",
                  AlertDescription::illegal_parameter);
            }

            // If we were otherwise going to accept early data we now need to
            // reject it. It's a little ugly to change our previous early data
            // decision, but doing it this way allows us to move the key
            // schedule forward as we do the key exchange.
            if (earlyDataType == EarlyDataType::Accepted) {
              earlyDataType = EarlyDataType::Rejected;
            }

            message_hash chloHash;
            chloHash.hash = handshakeContext->getHandshakeContext();
            handshakeContext =
                state.context()->getFactory()->makeHandshakeContext(cipher);
            handshakeContext->appendToTranscript(
                encodeHandshake(std::move(chloHash)));

            auto hrr = getHelloRetryRequest(
                version,
                cipher,
                *group,
                legacySessionId ? legacySessionId->clone() : nullptr,
                *handshakeContext);

            if (echStatus == ECHStatus::Accepted) {
              // Set up acceptance scheduler
              auto echScheduler =
                  state.context()->getFactory()->makeKeyScheduler(cipher);
              echScheduler->deriveEarlySecret(folly::range(chlo.random));
              // Add acceptance extension
              ech::setAcceptConfirmation(
                  hrr, handshakeContext->clone(), std::move(echScheduler));
            }

            auto encodedHelloRetryRequest = encodeHandshake(std::move(hrr));
            handshakeContext->appendToTranscript(encodedHelloRetryRequest);

            WriteToSocket serverFlight;
            serverFlight.contents.emplace_back(
                state.writeRecordLayer()->writeHandshake(
                    std::move(encodedHelloRetryRequest)));

            if (legacySessionId && !legacySessionId->empty()) {
              TLSContent writeCCS;
              writeCCS.encryptionLevel = EncryptionLevel::Plaintext;
              writeCCS.contentType = ContentType::change_cipher_spec;
              writeCCS.data = folly::IOBuf::wrapBuffer(FakeChangeCipherSpec);
              serverFlight.contents.emplace_back(std::move(writeCCS));
            }

            // Create a new record layer in case we need to skip early data.
            auto newReadRecordLayer =
                state.context()->getFactory()->makePlaintextReadRecordLayer();
            newReadRecordLayer->setSkipEncryptedRecords(
                earlyDataType == EarlyDataType::Rejected);

            return SemiFuture<Actions>(actions(
                MutateState([handshakeContext = std::move(handshakeContext),
                             version,
                             cipher,
                             group,
                             earlyDataType,
                             replayCacheResult,
                             newReadRecordLayer = std::move(newReadRecordLayer),
                             echStatus,
                             echState =
                                 std::move(echState)](State& newState) mutable {
                  // Save some information about the current state to be
                  // validated when we get the second client hello. We don't
                  // validate that the second client hello matches the first
                  // as strictly as we could according to the spec however.
                  newState.handshakeContext() = std::move(handshakeContext);
                  newState.version() = version;
                  newState.cipher() = cipher;
                  newState.group() = group;
                  newState.keyExchangeType() =
                      KeyExchangeType::HelloRetryRequest;
                  newState.earlyDataType() = earlyDataType;
                  newState.replayCacheResult() = replayCacheResult;
                  newState.readRecordLayer() = std::move(newReadRecordLayer);
                  newState.echStatus() = echStatus;
                  newState.echState() = std::move(echState);
                }),
                std::move(serverFlight),
                MutateState(&Transition<StateEnum::ExpectingClientHello>)));
          }

          if (state.keyExchangeType().has_value()) {
            keyExchangeType = *state.keyExchangeType();
          } else {
            keyExchangeType = KeyExchangeType::OneRtt;
          }

          // The exceptions in SemiFutures will be processed in
          // detail::processEvent.
          kex = state.context()->getFactory()->makeKeyExchange(
              *group, Factory::KeyExchangeMode::Server);
          kexResultFuture =
              doKexFuture(kex.get(), std::move(clientShare.value()));
        } else {
          keyExchangeType = KeyExchangeType::None;
        }

        return runOnCallerIfComplete(
            state.executor(),
            std::move(kexResultFuture),
            [&state,
             scheduler = std::move(scheduler),
             handshakeContext = std::move(handshakeContext),
             cipher,
             group,
             echStatus,
             echState = std::move(echState),
             earlyReadRecordLayer = std::move(earlyReadRecordLayer),
             earlyReadSecretAvailable = std::move(earlyReadSecretAvailable),
             earlyExporterMaster = std::move(earlyExporterMaster),
             pskType,
             pskMode,
             version,
             keyExchangeType,
             earlyDataType,
             replayCacheResult,
             alpn = std::move(alpn),
             clockSkew,
             appToken = std::move(appToken),
             legacySessionId = std::move(legacySessionId),
             handshakeTime,
             chlo = std::move(chlo),
             cookieState = std::move(cookieState),
             resState = std::move(resState),
             // Hold kex until the doKexFuture finished.
             kex = std::move(kex)](
                Optional<AsyncKeyExchange::DoKexResult> kexResult) mutable {
              Optional<Buf> serverShare;
              if (kexResult.hasValue()) {
                serverShare = std::move(kexResult.value().ourKeyShare);
                scheduler->deriveHandshakeSecret(
                    kexResult.value().sharedSecret->coalesce());
              } else {
                DCHECK(keyExchangeType == KeyExchangeType::None);
                scheduler->deriveHandshakeSecret();
              }
              std::vector<Extension> additionalExtensions;
              if (state.extensions()) {
                additionalExtensions = state.extensions()->getExtensions(chlo);
              }

              if (state.group().has_value() &&
                  (!group || *group != *state.group())) {
                throw FizzException(
                    "group mismatch with previous negotiation",
                    AlertDescription::illegal_parameter);
              }

              // Cookies are not required to have already negotiated the group
              // but if they did it must match (psk_ke is still allowed as we
              // may not know if we are accepting the psk when sending the
              // cookie).
              if (cookieState && cookieState->group && group &&
                  *group != *cookieState->group) {
                throw FizzException(
                    "group mismatch with cookie",
                    AlertDescription::illegal_parameter);
              }

              auto serverHello = getServerHello(
                  version,
                  state.context()->getFactory()->makeRandom(),
                  cipher,
                  resState.has_value(),
                  group,
                  std::move(serverShare),
                  legacySessionId ? legacySessionId->clone() : nullptr);

              folly::Optional<std::vector<ech::ECHConfig>> echRetryConfigs;
              if (echStatus == ECHStatus::Accepted) {
                // Set up acceptance scheduler
                auto echScheduler =
                    state.context()->getFactory()->makeKeyScheduler(cipher);
                echScheduler->deriveEarlySecret(folly::range(chlo.random));
                // Add acceptance extension
                ech::setAcceptConfirmation(
                    serverHello,
                    handshakeContext->clone(),
                    std::move(echScheduler));
              } else if (echStatus == ECHStatus::Rejected) {
                auto decrypter = state.context()->getECHDecrypter();
                echRetryConfigs = decrypter->getRetryConfigs();
              }

              auto encodedServerHello = encodeHandshake(std::move(serverHello));
              handshakeContext->appendToTranscript(encodedServerHello);

              // Derive handshake keys.
              auto handshakeWriteRecordLayer =
                  state.context()->getFactory()->makeEncryptedWriteRecordLayer(
                      EncryptionLevel::Handshake);
              handshakeWriteRecordLayer->setProtocolVersion(version);
              auto handshakeWriteSecret = scheduler->getSecret(
                  HandshakeSecrets::ServerHandshakeTraffic,
                  handshakeContext->getHandshakeContext()->coalesce());
              Protocol::setAead(
                  *handshakeWriteRecordLayer,
                  cipher,
                  folly::range(handshakeWriteSecret.secret),
                  *state.context()->getFactory(),
                  *scheduler);

              auto handshakeReadRecordLayer =
                  state.context()->getFactory()->makeEncryptedReadRecordLayer(
                      EncryptionLevel::Handshake);
              handshakeReadRecordLayer->setProtocolVersion(version);
              handshakeReadRecordLayer->setSkipFailedDecryption(
                  earlyDataType == EarlyDataType::Rejected);
              auto handshakeReadSecret = scheduler->getSecret(
                  HandshakeSecrets::ClientHandshakeTraffic,
                  handshakeContext->getHandshakeContext()->coalesce());
              Protocol::setAead(
                  *handshakeReadRecordLayer,
                  cipher,
                  folly::range(handshakeReadSecret.secret),
                  *state.context()->getFactory(),
                  *scheduler);
              auto clientHandshakeSecret =
                  folly::IOBuf::copyBuffer(handshakeReadSecret.secret);

              auto encodedEncryptedExt = getEncryptedExt(
                  *handshakeContext,
                  alpn,
                  earlyDataType,
                  std::move(echRetryConfigs),
                  std::move(additionalExtensions));

              /*
               * Determine we are requesting client auth.
               * If yes, add CertificateRequest to handshake write and
               * transcript.
               */
              bool requestClientAuth = state.context()->getClientAuthMode() !=
                      ClientAuthMode::None &&
                  !resState;
              Optional<Buf> encodedCertRequest;
              if (requestClientAuth) {
                encodedCertRequest = getCertificateRequest(
                    state.context()->getSupportedSigSchemes(),
                    state.context()->getClientCertVerifier().get(),
                    *handshakeContext);
              }

              /*
               * Set the cert and signature scheme we are using.
               * If sending new cert, add Certificate to handshake write and
               * transcript.
               */
              Optional<Buf> encodedCertificate;
              SemiFuture<Optional<Buf>> signature = folly::none;
              Optional<SignatureScheme> sigScheme;
              Optional<std::shared_ptr<const Cert>> serverCert;
              std::shared_ptr<const Cert> clientCert;
              Optional<CertificateCompressionAlgorithm> certCompressionAlgo;
              if (!resState) { // TODO or reauth
                std::shared_ptr<const SelfCert> originalSelfCert;
                std::tie(originalSelfCert, sigScheme) =
                    chooseCert(*state.context(), chlo);

                std::tie(encodedCertificate, certCompressionAlgo) =
                    getCertificate(
                        originalSelfCert,
                        *state.context(),
                        chlo,
                        *handshakeContext);

                auto toBeSigned = handshakeContext->getHandshakeContext();
                auto asyncSelfCert =
                    dynamic_cast<const AsyncSelfCert*>(originalSelfCert.get());
                if (asyncSelfCert) {
                  signature = asyncSelfCert->signFuture(
                      *sigScheme,
                      CertificateVerifyContext::Server,
                      std::move(toBeSigned));
                } else {
                  signature = folly::makeSemiFuture<Optional<Buf>>(
                      originalSelfCert->sign(
                          *sigScheme,
                          CertificateVerifyContext::Server,
                          toBeSigned->coalesce()));
                }
                serverCert = std::move(originalSelfCert);
              } else {
                serverCert = std::move(resState->serverCert);
                clientCert = std::move(resState->clientCert);
              }

              auto clientRandom = std::move(chlo.random);

              return runOnCallerIfComplete(
                  state.executor(),
                  std::move(signature),
                  [&state,
                   scheduler = std::move(scheduler),
                   handshakeContext = std::move(handshakeContext),
                   cipher,
                   clientRandom = std::move(clientRandom),
                   group,
                   echStatus,
                   echState = std::move(echState),
                   encodedServerHello = std::move(encodedServerHello),
                   handshakeWriteRecordLayer =
                       std::move(handshakeWriteRecordLayer),
                   handshakeWriteSecret = std::move(handshakeWriteSecret),
                   handshakeReadRecordLayer =
                       std::move(handshakeReadRecordLayer),
                   handshakeReadSecret = std::move(handshakeReadSecret),
                   earlyReadRecordLayer = std::move(earlyReadRecordLayer),
                   earlyReadSecretAvailable =
                       std::move(earlyReadSecretAvailable),
                   earlyExporterMaster = std::move(earlyExporterMaster),
                   clientHandshakeSecret = std::move(clientHandshakeSecret),
                   encodedEncryptedExt = std::move(encodedEncryptedExt),
                   encodedCertificate = std::move(encodedCertificate),
                   encodedCertRequest = std::move(encodedCertRequest),
                   requestClientAuth,
                   pskType,
                   pskMode,
                   sigScheme,
                   version,
                   keyExchangeType,
                   earlyDataType,
                   replayCacheResult,
                   serverCert = std::move(serverCert),
                   clientCert = std::move(clientCert),
                   alpn = std::move(alpn),
                   clockSkew,
                   appToken = std::move(appToken),
                   legacySessionId = std::move(legacySessionId),
                   serverCertCompAlgo = certCompressionAlgo,
                   handshakeTime](Optional<Buf> sig) mutable {
                    Optional<Buf> encodedCertificateVerify;
                    if (sig) {
                      encodedCertificateVerify = getCertificateVerify(
                          *sigScheme, std::move(*sig), *handshakeContext);
                    }

                    auto encodedFinished = Protocol::getFinished(
                        folly::range(handshakeWriteSecret.secret),
                        *handshakeContext);

                    folly::IOBufQueue combined;
                    if (encodedCertificate) {
                      if (encodedCertRequest) {
                        combined.append(std::move(encodedEncryptedExt));
                        combined.append(std::move(*encodedCertRequest));
                        combined.append(std::move(*encodedCertificate));
                        combined.append(std::move(*encodedCertificateVerify));
                        combined.append(std::move(encodedFinished));
                      } else {
                        combined.append(std::move(encodedEncryptedExt));
                        combined.append(std::move(*encodedCertificate));
                        combined.append(std::move(*encodedCertificateVerify));
                        combined.append(std::move(encodedFinished));
                      }
                    } else {
                      combined.append(std::move(encodedEncryptedExt));
                      combined.append(std::move(encodedFinished));
                    }

                    // Some middleboxes appear to break if the first encrypted
                    // record is larger than ~1300 bytes (likely if it does not
                    // fit in the first packet).
                    auto serverEncrypted =
                        handshakeWriteRecordLayer->writeHandshake(
                            combined.splitAtMost(1000));
                    if (!combined.empty()) {
                      auto splitRecord =
                          handshakeWriteRecordLayer->writeHandshake(
                              combined.move());
                      // Split record must have the same encryption level as the
                      // main handshake.
                      DCHECK(
                          splitRecord.encryptionLevel ==
                          serverEncrypted.encryptionLevel);
                      serverEncrypted.data->prependChain(
                          std::move(splitRecord.data));
                    }

                    WriteToSocket serverFlight;
                    serverFlight.contents.emplace_back(
                        state.writeRecordLayer()->writeHandshake(
                            std::move(encodedServerHello)));
                    if (legacySessionId && !legacySessionId->empty()) {
                      TLSContent ccsWrite;
                      ccsWrite.encryptionLevel = EncryptionLevel::Plaintext;
                      ccsWrite.contentType = ContentType::change_cipher_spec;
                      ccsWrite.data =
                          folly::IOBuf::wrapBuffer(FakeChangeCipherSpec);
                      serverFlight.contents.emplace_back(std::move(ccsWrite));
                    }
                    serverFlight.contents.emplace_back(
                        std::move(serverEncrypted));

                    scheduler->deriveMasterSecret();
                    auto clientFinishedContext =
                        handshakeContext->getHandshakeContext();
                    auto exporterMasterVector = scheduler->getSecret(
                        MasterSecrets::ExporterMaster,
                        clientFinishedContext->coalesce());
                    auto exporterMaster = folly::IOBuf::copyBuffer(
                        folly::range(exporterMasterVector.secret));

                    scheduler->deriveAppTrafficSecrets(
                        clientFinishedContext->coalesce());
                    auto appTrafficWriteRecordLayer =
                        state.context()
                            ->getFactory()
                            ->makeEncryptedWriteRecordLayer(
                                EncryptionLevel::AppTraffic);
                    appTrafficWriteRecordLayer->setProtocolVersion(version);
                    auto writeSecret = scheduler->getSecret(
                        AppTrafficSecrets::ServerAppTraffic);
                    Protocol::setAead(
                        *appTrafficWriteRecordLayer,
                        cipher,
                        folly::range(writeSecret.secret),
                        *state.context()->getFactory(),
                        *scheduler);

                    // If we have previously dealt with early data (before a
                    // HelloRetryRequest), don't overwrite the previous result.
                    auto earlyDataTypeSave = state.earlyDataType()
                        ? *state.earlyDataType()
                        : earlyDataType;

                    SecretAvailable handshakeReadSecretAvailable(
                        std::move(handshakeReadSecret));
                    SecretAvailable handshakeWriteSecretAvailable(
                        std::move(handshakeWriteSecret));
                    SecretAvailable appWriteSecretAvailable(
                        std::move(writeSecret));

                    // Save all the necessary state except for the read record
                    // layer, which is done separately as it varies if early
                    // data was accepted.
                    MutateState saveState(
                        [appTrafficWriteRecordLayer =
                             std::move(appTrafficWriteRecordLayer),
                         handshakeContext = std::move(handshakeContext),
                         scheduler = std::move(scheduler),
                         exporterMaster = std::move(exporterMaster),
                         serverCert = std::move(serverCert),
                         clientCert = std::move(clientCert),
                         cipher,
                         group,
                         sigScheme,
                         clientHandshakeSecret =
                             std::move(clientHandshakeSecret),
                         pskType,
                         pskMode,
                         version,
                         keyExchangeType,
                         alpn = std::move(alpn),
                         earlyDataTypeSave,
                         replayCacheResult,
                         clockSkew,
                         appToken = std::move(appToken),
                         serverCertCompAlgo,
                         echStatus,
                         echState = std::move(echState),
                         clientRandom = std::move(clientRandom),
                         handshakeTime = std::move(handshakeTime)](
                            State& newState) mutable {
                          newState.writeRecordLayer() =
                              std::move(appTrafficWriteRecordLayer);
                          newState.handshakeContext() =
                              std::move(handshakeContext);
                          newState.keyScheduler() = std::move(scheduler);
                          newState.exporterMasterSecret() =
                              std::move(exporterMaster);
                          newState.serverCert() = std::move(*serverCert);
                          newState.clientCert() = std::move(clientCert);
                          newState.version() = version;
                          newState.cipher() = cipher;
                          newState.group() = group;
                          newState.sigScheme() = sigScheme;
                          newState.clientHandshakeSecret() =
                              std::move(clientHandshakeSecret);
                          newState.pskType() = pskType;
                          newState.pskMode() = pskMode;
                          newState.keyExchangeType() = keyExchangeType;
                          newState.earlyDataType() = earlyDataTypeSave;
                          newState.replayCacheResult() = replayCacheResult;
                          newState.alpn() = std::move(alpn);
                          newState.clientClockSkew() = clockSkew;
                          newState.appToken() = std::move(appToken);
                          newState.serverCertCompAlgo() = serverCertCompAlgo;
                          newState.handshakeTime() = std::move(handshakeTime);
                          newState.clientRandom() = std::move(clientRandom);
                          newState.echStatus() = echStatus;
                          newState.echState() = std::move(echState);
                        });

                    if (earlyDataType == EarlyDataType::Accepted) {
                      if (state.context()->getOmitEarlyRecordLayer()) {
                        return actions(
                            MutateState([handshakeReadRecordLayer = std::move(
                                             handshakeReadRecordLayer),
                                         earlyExporterMaster =
                                             std::move(earlyExporterMaster)](
                                            State& newState) mutable {
                              newState.readRecordLayer() =
                                  std::move(handshakeReadRecordLayer);
                              newState.earlyExporterMasterSecret() =
                                  std::move(earlyExporterMaster);
                            }),
                            std::move(saveState),
                            std::move(*earlyReadSecretAvailable),
                            std::move(handshakeReadSecretAvailable),
                            std::move(handshakeWriteSecretAvailable),
                            std::move(appWriteSecretAvailable),
                            std::move(serverFlight),
                            MutateState(
                                &Transition<StateEnum::ExpectingFinished>),
                            ReportEarlyHandshakeSuccess());

                      } else {
                        return actions(
                            MutateState([handshakeReadRecordLayer = std::move(
                                             handshakeReadRecordLayer),
                                         earlyReadRecordLayer =
                                             std::move(earlyReadRecordLayer),
                                         earlyExporterMaster =
                                             std::move(earlyExporterMaster)](
                                            State& newState) mutable {
                              newState.readRecordLayer() =
                                  std::move(earlyReadRecordLayer);
                              newState.handshakeReadRecordLayer() =
                                  std::move(handshakeReadRecordLayer);
                              newState.earlyExporterMasterSecret() =
                                  std::move(earlyExporterMaster);
                            }),
                            std::move(saveState),
                            std::move(*earlyReadSecretAvailable),
                            std::move(handshakeReadSecretAvailable),
                            std::move(handshakeWriteSecretAvailable),
                            std::move(appWriteSecretAvailable),
                            std::move(serverFlight),
                            MutateState(
                                &Transition<StateEnum::AcceptingEarlyData>),
                            ReportEarlyHandshakeSuccess());
                      }
                    } else {
                      auto transition = requestClientAuth
                          ? Transition<StateEnum::ExpectingCertificate>
                          : Transition<StateEnum::ExpectingFinished>;
                      return actions(
                          MutateState([handshakeReadRecordLayer =
                                           std::move(handshakeReadRecordLayer)](
                                          State& newState) mutable {
                            newState.readRecordLayer() =
                                std::move(handshakeReadRecordLayer);
                          }),
                          std::move(saveState),
                          std::move(handshakeReadSecretAvailable),
                          std::move(handshakeWriteSecretAvailable),
                          std::move(appWriteSecretAvailable),
                          std::move(serverFlight),
                          MutateState(transition));
                    }
                  });
            });
      });
}

AsyncActions
EventHandler<ServerTypes, StateEnum::AcceptingEarlyData, Event::AppData>::
    handle(const State&, Param param) {
  auto& appData = *param.asAppData();

  return actions(DeliverAppData{std::move(appData.data)});
}

AsyncActions
EventHandler<ServerTypes, StateEnum::AcceptingEarlyData, Event::AppWrite>::
    handle(const State& state, Param param) {
  auto& appWrite = *param.asAppWrite();

  WriteToSocket write;
  write.token = appWrite.token;
  write.contents.emplace_back(state.writeRecordLayer()->writeAppData(
      std::move(appWrite.data), appWrite.aeadOptions));
  write.flags = appWrite.flags;

  return actions(std::move(write));
}

AsyncActions EventHandler<
    ServerTypes,
    StateEnum::AcceptingEarlyData,
    Event::EndOfEarlyData>::handle(const State& state, Param param) {
  auto& eoed = *param.asEndOfEarlyData();

  if (state.readRecordLayer()->hasUnparsedHandshakeData()) {
    throw FizzException(
        "data after eoed", AlertDescription::unexpected_message);
  }

  state.handshakeContext()->appendToTranscript(*eoed.originalEncoding);

  auto readRecordLayer = std::move(state.handshakeReadRecordLayer());

  return actions(
      MutateState([readRecordLayer =
                       std::move(readRecordLayer)](State& newState) mutable {
        newState.readRecordLayer() = std::move(readRecordLayer);
      }),
      MutateState(&Transition<StateEnum::ExpectingFinished>));
}

AsyncActions
EventHandler<ServerTypes, StateEnum::ExpectingFinished, Event::AppWrite>::
    handle(const State& state, Param param) {
  auto& appWrite = *param.asAppWrite();

  WriteToSocket write;
  write.token = appWrite.token;
  write.contents.emplace_back(state.writeRecordLayer()->writeAppData(
      std::move(appWrite.data), appWrite.aeadOptions));
  write.flags = appWrite.flags;

  return actions(std::move(write));
}

static WriteToSocket writeNewSessionTicket(
    const FizzServerContext& context,
    const WriteRecordLayer& recordLayer,
    std::chrono::seconds ticketLifetime,
    uint32_t ticketAgeAdd,
    Buf nonce,
    Buf ticket,
    ProtocolVersion version) {
  NewSessionTicket nst;
  nst.ticket_lifetime = ticketLifetime.count();
  nst.ticket_age_add = ticketAgeAdd;
  nst.ticket_nonce = std::move(nonce);
  nst.ticket = std::move(ticket);

  if (context.getAcceptEarlyData(version)) {
    TicketEarlyData early;
    early.max_early_data_size = context.getMaxEarlyDataSize();
    nst.extensions.push_back(encodeExtension(std::move(early)));
  }

  auto encodedNst = encodeHandshake(std::move(nst));
  WriteToSocket nstWrite;
  nstWrite.contents.emplace_back(
      recordLayer.writeHandshake(std::move(encodedNst)));
  return nstWrite;
}

static SemiFuture<Optional<WriteToSocket>> generateTicket(
    const State& state,
    const std::vector<uint8_t>& resumptionMasterSecret,
    Buf appToken = nullptr) {
  auto ticketCipher = state.context()->getTicketCipher();

  if (!ticketCipher || *state.pskType() == PskType::NotSupported) {
    return folly::none;
  }

  Buf resumptionSecret;
  auto ticketNonce = folly::IOBuf::create(0);
  resumptionSecret = state.keyScheduler()->getResumptionSecret(
      folly::range(resumptionMasterSecret), ticketNonce->coalesce());

  auto ticketAgeAdd = state.context()->getFactory()->makeTicketAgeAdd();
  ResumptionState resState;
  resState.version = *state.version();
  resState.cipher = *state.cipher();
  resState.resumptionSecret = std::move(resumptionSecret);
  resState.serverCert = state.serverCert();
  resState.clientCert = state.clientCert();
  resState.alpn = state.alpn();
  resState.ticketAgeAdd = ticketAgeAdd;
  resState.ticketIssueTime = state.context()->getClock().getCurrentTime();
  resState.appToken = std::move(appToken);
  resState.handshakeTime = *state.handshakeTime();

  auto ticketFuture = ticketCipher->encrypt(std::move(resState));
  return runOnCallerIfComplete(
      state.executor(),
      std::move(ticketFuture),
      [&state, ticketAgeAdd, ticketNonce = std::move(ticketNonce)](
          Optional<std::pair<Buf, std::chrono::seconds>> ticket) mutable
      -> Optional<WriteToSocket> {
        if (!ticket) {
          return folly::none;
        }
        return writeNewSessionTicket(
            *state.context(),
            *state.writeRecordLayer(),
            ticket->second,
            ticketAgeAdd,
            std::move(ticketNonce),
            std::move(ticket->first),
            *state.version());
      });
}

AsyncActions
EventHandler<ServerTypes, StateEnum::ExpectingCertificate, Event::Certificate>::
    handle(const State& state, Param param) {
  auto certMsg = std::move(*param.asCertificateMsg());

  state.handshakeContext()->appendToTranscript(*certMsg.originalEncoding);

  if (!certMsg.certificate_request_context->empty()) {
    throw FizzException(
        "certificate request context must be empty",
        AlertDescription::illegal_parameter);
  }

  std::vector<std::shared_ptr<const PeerCert>> clientCerts;
  bool leaf = true;
  for (auto& certEntry : certMsg.certificate_list) {
    // We don't request any extensions, so this ought to be empty
    if (!certEntry.extensions.empty()) {
      throw FizzException(
          "certificate extensions must be empty",
          AlertDescription::illegal_parameter);
    }

    clientCerts.emplace_back(state.context()->getFactory()->makePeerCert(
        std::move(certEntry), leaf));
    leaf = false;
  }

  if (clientCerts.empty()) {
    if (state.context()->getClientAuthMode() == ClientAuthMode::Optional) {
      VLOG(6) << "Client authentication not sent";
      return actions(
          MutateState([](State& newState) {
            newState.unverifiedCertChain() = folly::none;
          }),
          MutateState(&Transition<StateEnum::ExpectingFinished>));
    } else {
      throw FizzException(
          "certificate requested but none received",
          AlertDescription::certificate_required);
    }
  } else {
    return actions(
        MutateState([certs = std::move(clientCerts)](State& newState) mutable {
          newState.unverifiedCertChain() = std::move(certs);
        }),
        MutateState(&Transition<StateEnum::ExpectingCertificateVerify>));
  }
}

AsyncActions EventHandler<
    ServerTypes,
    StateEnum::ExpectingCertificateVerify,
    Event::CertificateVerify>::handle(const State& state, Param param) {
  auto certVerify = std::move(*param.asCertificateVerify());

  if (std::find(
          state.context()->getSupportedSigSchemes().begin(),
          state.context()->getSupportedSigSchemes().end(),
          certVerify.algorithm) ==
      state.context()->getSupportedSigSchemes().end()) {
    throw FizzException(
        folly::to<std::string>(
            "client chose unsupported sig scheme: ",
            toString(certVerify.algorithm)),
        AlertDescription::handshake_failure);
  }

  const auto& certs = *state.unverifiedCertChain();
  auto leafCert = certs.front();
  leafCert->verify(
      certVerify.algorithm,
      CertificateVerifyContext::Client,
      state.handshakeContext()->getHandshakeContext()->coalesce(),
      certVerify.signature->coalesce());

  std::shared_ptr<const Cert> newCert;

  try {
    const auto& verifier = state.context()->getClientCertVerifier();
    if (verifier) {
      if (auto verifiedCert = verifier->verify(certs)) {
        newCert = std::move(verifiedCert);
      } else {
        newCert = std::move(leafCert);
      }
    } else {
      newCert = std::move(leafCert);
    }
  } catch (const FizzException&) {
    throw;
  } catch (const std::exception& e) {
    throw FizzVerificationException(
        folly::to<std::string>("client certificate failure: ", e.what()),
        AlertDescription::bad_certificate);
  }

  state.handshakeContext()->appendToTranscript(*certVerify.originalEncoding);

  return actions(
      MutateState([cert = std::move(newCert)](State& newState) {
        newState.unverifiedCertChain() = folly::none;
        newState.clientCert() = std::move(cert);
      }),
      MutateState(&Transition<StateEnum::ExpectingFinished>));
}

AsyncActions
EventHandler<ServerTypes, StateEnum::ExpectingFinished, Event::Finished>::
    handle(const State& state, Param param) {
  auto& finished = *param.asFinished();

  auto expectedFinished = state.handshakeContext()->getFinishedData(
      state.clientHandshakeSecret()->coalesce());
  if (!CryptoUtils::equal(
          expectedFinished->coalesce(), finished.verify_data->coalesce())) {
    throw FizzException("client finished verify failure", folly::none);
  }

  if (state.readRecordLayer()->hasUnparsedHandshakeData()) {
    throw FizzException("data after finished", folly::none);
  }

  auto readRecordLayer =
      state.context()->getFactory()->makeEncryptedReadRecordLayer(
          EncryptionLevel::AppTraffic);
  readRecordLayer->setProtocolVersion(*state.version());
  auto readSecret =
      state.keyScheduler()->getSecret(AppTrafficSecrets::ClientAppTraffic);
  Protocol::setAead(
      *readRecordLayer,
      *state.cipher(),
      folly::range(readSecret.secret),
      *state.context()->getFactory(),
      *state.keyScheduler());

  state.handshakeContext()->appendToTranscript(*finished.originalEncoding);

  auto resumptionMasterSecret =
      state.keyScheduler()
          ->getSecret(
              MasterSecrets::ResumptionMaster,
              state.handshakeContext()->getHandshakeContext()->coalesce())
          .secret;
  state.keyScheduler()->clearMasterSecret();

  MutateState saveState([readRecordLayer = std::move(readRecordLayer),
                         resumptionMasterSecret](State& newState) mutable {
    newState.readRecordLayer() = std::move(readRecordLayer);
    newState.resumptionMasterSecret() = std::move(resumptionMasterSecret);
  });

  SecretAvailable appReadTrafficSecretAvailable(std::move(readSecret));

  if (!state.context()->getSendNewSessionTicket()) {
    return actions(
        std::move(saveState),
        std::move(appReadTrafficSecretAvailable),
        MutateState(&Transition<StateEnum::AcceptingData>),
        ReportHandshakeSuccess());
  } else {
    auto ticketFuture = generateTicket(state, resumptionMasterSecret);
    return runOnCallerIfComplete(
        state.executor(),
        std::move(ticketFuture),
        [saveState = std::move(saveState),
         appReadTrafficSecretAvailable =
             std::move(appReadTrafficSecretAvailable)](
            Optional<WriteToSocket> nstWrite) mutable {
          if (!nstWrite) {
            return actions(
                std::move(saveState),
                MutateState(&Transition<StateEnum::AcceptingData>),
                std::move(appReadTrafficSecretAvailable),
                ReportHandshakeSuccess());
          }

          return actions(
              std::move(saveState),
              MutateState(&Transition<StateEnum::AcceptingData>),
              std::move(appReadTrafficSecretAvailable),
              std::move(*nstWrite),
              ReportHandshakeSuccess());
        });
  }
}

AsyncActions EventHandler<
    ServerTypes,
    StateEnum::AcceptingData,
    Event::WriteNewSessionTicket>::handle(const State& state, Param param) {
  auto& writeNewSessionTicket = *param.asWriteNewSessionTicket();
  auto ticketFuture = generateTicket(
      state,
      state.resumptionMasterSecret(),
      std::move(writeNewSessionTicket.appToken));
  return runOnCallerIfComplete(
      state.executor(),
      std::move(ticketFuture),
      [](Optional<WriteToSocket> nstWrite) {
        if (!nstWrite) {
          return Actions();
        }
        return actions(std::move(*nstWrite));
      });
}

AsyncActions
EventHandler<ServerTypes, StateEnum::AcceptingData, Event::AppData>::handle(
    const State& /*state*/,
    Param param) {
  auto& appData = *param.asAppData();

  return actions(DeliverAppData{std::move(appData.data)});
}

AsyncActions
EventHandler<ServerTypes, StateEnum::AcceptingData, Event::AppWrite>::handle(
    const State& state,
    Param param) {
  auto& appWrite = *param.asAppWrite();

  WriteToSocket write;
  write.token = appWrite.token;
  write.contents.emplace_back(state.writeRecordLayer()->writeAppData(
      std::move(appWrite.data), appWrite.aeadOptions));
  write.flags = appWrite.flags;

  return actions(std::move(write));
}

AsyncActions EventHandler<
    ServerTypes,
    StateEnum::AcceptingData,
    Event::KeyUpdateInitiation>::handle(const State& state, Param param) {
  if (state.readRecordLayer()->hasUnparsedHandshakeData()) {
    throw FizzException("data after key_update", folly::none);
  }
  auto& keyUpdateInitiation = *param.asKeyUpdateInitiation();
  auto encodedKeyUpdated =
      Protocol::getKeyUpdated(keyUpdateInitiation.request_update);
  WriteToSocket write;
  write.contents.emplace_back(
      state.writeRecordLayer()->writeHandshake(std::move(encodedKeyUpdated)));

  state.keyScheduler()->serverKeyUpdate();

  auto writeRecordLayer =
      state.context()->getFactory()->makeEncryptedWriteRecordLayer(
          EncryptionLevel::AppTraffic);
  writeRecordLayer->setProtocolVersion(*state.version());
  auto writeSecret =
      state.keyScheduler()->getSecret(AppTrafficSecrets::ServerAppTraffic);
  Protocol::setAead(
      *writeRecordLayer,
      *state.cipher(),
      folly::range(writeSecret.secret),
      *state.context()->getFactory(),
      *state.keyScheduler());

  return actions(
      MutateState([wRecordLayer =
                       std::move(writeRecordLayer)](State& newState) mutable {
        newState.writeRecordLayer() = std::move(wRecordLayer);
      }),
      SecretAvailable(std::move(writeSecret)),
      std::move(write));
}

AsyncActions
EventHandler<ServerTypes, StateEnum::AcceptingData, Event::KeyUpdate>::handle(
    const State& state,
    Param param) {
  auto& keyUpdate = *param.asKeyUpdate();

  if (state.readRecordLayer()->hasUnparsedHandshakeData()) {
    throw FizzException("data after key_update", folly::none);
  }

  state.keyScheduler()->clientKeyUpdate();
  auto readRecordLayer =
      state.context()->getFactory()->makeEncryptedReadRecordLayer(
          EncryptionLevel::AppTraffic);
  readRecordLayer->setProtocolVersion(*state.version());
  auto readSecret =
      state.keyScheduler()->getSecret(AppTrafficSecrets::ClientAppTraffic);
  Protocol::setAead(
      *readRecordLayer,
      *state.cipher(),
      folly::range(readSecret.secret),
      *state.context()->getFactory(),
      *state.keyScheduler());

  if (keyUpdate.request_update == KeyUpdateRequest::update_not_requested) {
    return actions(
        MutateState([rRecordLayer =
                         std::move(readRecordLayer)](State& newState) mutable {
          newState.readRecordLayer() = std::move(rRecordLayer);
        }),
        SecretAvailable(std::move(readSecret)));
  }

  auto encodedKeyUpdated =
      Protocol::getKeyUpdated(KeyUpdateRequest::update_not_requested);
  WriteToSocket write;
  write.contents.emplace_back(
      state.writeRecordLayer()->writeHandshake(std::move(encodedKeyUpdated)));

  state.keyScheduler()->serverKeyUpdate();

  auto writeRecordLayer =
      state.context()->getFactory()->makeEncryptedWriteRecordLayer(
          EncryptionLevel::AppTraffic);
  writeRecordLayer->setProtocolVersion(*state.version());
  auto writeSecret =
      state.keyScheduler()->getSecret(AppTrafficSecrets::ServerAppTraffic);
  Protocol::setAead(
      *writeRecordLayer,
      *state.cipher(),
      folly::range(writeSecret.secret),
      *state.context()->getFactory(),
      *state.keyScheduler());

  return actions(
      MutateState([rRecordLayer = std::move(readRecordLayer),
                   wRecordLayer =
                       std::move(writeRecordLayer)](State& newState) mutable {
        newState.readRecordLayer() = std::move(rRecordLayer);
        newState.writeRecordLayer() = std::move(wRecordLayer);
      }),
      SecretAvailable(std::move(writeSecret)),
      SecretAvailable(std::move(readSecret)),
      std::move(write));
}

AsyncActions
EventHandler<ServerTypes, StateEnum::AcceptingData, Event::CloseNotify>::handle(
    const State& state,
    Param param) {
  ensureNoUnparsedHandshakeData(state, Event::CloseNotify);
  auto& closenotify = *param.asCloseNotify();
  auto eod = EndOfData(std::move(closenotify.ignoredPostCloseData));

  MutateState clearRecordLayers([](State& newState) {
    newState.writeRecordLayer() = nullptr;
    newState.readRecordLayer() = nullptr;
  });

  WriteToSocket write;
  write.contents.emplace_back(state.writeRecordLayer()->writeAlert(
      Alert(AlertDescription::close_notify)));
  return actions(
      std::move(write),
      std::move(clearRecordLayers),
      MutateState(&Transition<StateEnum::Closed>),
      std::move(eod));
}

AsyncActions
EventHandler<ServerTypes, StateEnum::ExpectingCloseNotify, Event::CloseNotify>::
    handle(const State& state, Param param) {
  ensureNoUnparsedHandshakeData(state, Event::CloseNotify);
  auto& closenotify = *param.asCloseNotify();
  auto eod = EndOfData(std::move(closenotify.ignoredPostCloseData));

  MutateState clearRecordLayers([](State& newState) {
    newState.readRecordLayer() = nullptr;
    newState.writeRecordLayer() = nullptr;
  });
  return actions(
      std::move(clearRecordLayers),
      MutateState(&Transition<StateEnum::Closed>),
      std::move(eod));
}

} // namespace sm
} // namespace fizz
