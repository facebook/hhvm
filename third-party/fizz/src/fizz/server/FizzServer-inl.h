/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/util/Workarounds.h>
#include <folly/Overload.h>

namespace fizz {
namespace server {

template <typename ActionMoveVisitor, typename SM>
void FizzServer<ActionMoveVisitor, SM>::accept(
    folly::Executor* executor,
    std::shared_ptr<const FizzServerContext> context,
    std::shared_ptr<ServerExtensions> extensions) {
  checkV2Hello_ = context->getVersionFallbackEnabled();
  this->addProcessingActions(this->machine_.processAccept(
      this->state_, executor, std::move(context), std::move(extensions)));
}

template <typename ActionMoveVisitor, typename SM>
void FizzServer<ActionMoveVisitor, SM>::newTransportData() {
  // If the first data we receive looks like an SSLv2 Client Hello we trigger
  // fallback immediately. This uses the same check as OpenSSL, and OpenSSL
  // does not allow extensions in an SSLv2 Client Hello, so this should not
  // add additional downgrade concerns.
  if (checkV2Hello_) {
    if (!this->actionProcessing() &&
        looksLikeV2ClientHello(this->transportReadBuf_)) {
      VLOG(3) << "Attempting fallback due to V2 ClientHello";
      AttemptVersionFallback fallback;
      fallback.clientHello = this->transportReadBuf_.move();
      return this->addProcessingActions(detail::actions(
          MutateState(
              [](State& newState) { newState.state() = StateEnum::Error; }),
          std::move(fallback)));
    }
    checkV2Hello_ = false;
  }

  FizzBase<FizzServer<ActionMoveVisitor, SM>, ActionMoveVisitor, SM>::
      newTransportData();
}

template <typename ActionMoveVisitor, typename SM>
Buf FizzServer<ActionMoveVisitor, SM>::getEarlyEkm(
    const Factory& factory,
    folly::StringPiece label,
    const Buf& context,
    uint16_t length) const {
  if (!this->state_.earlyExporterMasterSecret()) {
    throw std::runtime_error("early ekm not available");
  }
  return Exporter::getExportedKeyingMaterial(
      factory,
      *this->state_.cipher(),
      (*this->state_.earlyExporterMasterSecret())->coalesce(),
      label,
      context ? context->clone() : nullptr,
      length);
}

template <typename ActionMoveVisitor, typename SM>
void FizzServer<ActionMoveVisitor, SM>::startActions(AsyncActions actions) {
  folly::variant_match(
      actions,
      ::fizz::detail::result_type<void>(),
      [this](folly::SemiFuture<Actions>& futureActions) {
        if (futureActions.isReady()) {
          auto result = std::move(futureActions).getTry();
          if (result.hasValue()) {
            this->processActions(result.value());
          }
        } else {
          std::move(futureActions)
              .via(this->state_.executor())
              .thenValueInline(
                  [this](Actions&& a) { this->processActions(a); });
        }
      },
      [this](Actions& immediateActions) {
        this->processActions(immediateActions);
      });
}

template <typename ActionMoveVisitor, typename SM>
void FizzServer<ActionMoveVisitor, SM>::visitActions(
    typename SM::CompletedActions& actions) {
  for (auto& action : actions) {
    switch (action.type()) {
      case Action::Type::DeliverAppData_E:
        this->visitor_(*action.asDeliverAppData());
        break;
      case Action::Type::WriteToSocket_E:
        this->visitor_(*action.asWriteToSocket());
        break;
      case Action::Type::ReportHandshakeSuccess_E:
        this->visitor_(*action.asReportHandshakeSuccess());
        break;
      case Action::Type::ReportEarlyHandshakeSuccess_E:
        this->visitor_(*action.asReportEarlyHandshakeSuccess());
        break;
      case Action::Type::ReportError_E:
        this->visitor_(*action.asReportError());
        break;
      case Action::Type::EndOfData_E:
        this->visitor_(*action.asEndOfData());
        break;
      case Action::Type::MutateState_E:
        this->visitor_(*action.asMutateState());
        break;
      case Action::Type::WaitForData_E:
        this->visitor_(*action.asWaitForData());
        break;
      case Action::Type::AttemptVersionFallback_E:
        this->visitor_(*action.asAttemptVersionFallback());
        break;
      case Action::Type::SecretAvailable_E:
        this->visitor_(*action.asSecretAvailable());
        break;
    }
  }
}
} // namespace server
} // namespace fizz
