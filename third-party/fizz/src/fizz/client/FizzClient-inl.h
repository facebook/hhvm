/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

namespace fizz {
namespace client {

template <typename ActionMoveVisitor, typename SM>
void FizzClient<ActionMoveVisitor, SM>::connect(
    std::shared_ptr<const FizzClientContext> context,
    std::shared_ptr<const CertificateVerifier> verifier,
    folly::Optional<std::string> sni,
    folly::Optional<CachedPsk> cachedPsk,
    folly::Optional<std::vector<ech::ECHConfig>> echConfigs,
    const std::shared_ptr<ClientExtensions>& extensions) {
  this->addProcessingActions(this->machine_.processConnect(
      this->state_,
      std::move(context),
      std::move(verifier),
      std::move(sni),
      std::move(cachedPsk),
      extensions,
      std::move(echConfigs)));
}

template <typename ActionMoveVisitor, typename SM>
void FizzClient<ActionMoveVisitor, SM>::connect(
    std::shared_ptr<const FizzClientContext> context,
    folly::Optional<std::string> hostname) {
  const auto pskIdentity = hostname;
  connect(
      std::move(context),
      std::make_shared<DefaultCertificateVerifier>(VerificationContext::Client),
      std::move(hostname),
      std::move(pskIdentity));
}

template <typename ActionMoveVisitor, typename SM>
Buf FizzClient<ActionMoveVisitor, SM>::getEarlyEkm(
    const Factory& factory,
    folly::StringPiece label,
    const Buf& context,
    uint16_t length) const {
  if (!this->state_.earlyDataParams()) {
    throw std::runtime_error("early ekm not available");
  }
  return Exporter::getExportedKeyingMaterial(
      factory,
      this->state_.earlyDataParams()->cipher,
      this->state_.earlyDataParams()->earlyExporterSecret->coalesce(),
      label,
      context ? context->clone() : nullptr,
      length);
}

template <typename ActionMoveVisitor, typename SM>
void FizzClient<ActionMoveVisitor, SM>::startActions(Actions actions) {
  this->processActions(actions);
}

template <typename ActionMoveVisitor, typename SM>
void FizzClient<ActionMoveVisitor, SM>::visitActions(
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
      case Action::Type::ReportEarlyWriteFailed_E:
        this->visitor_(*action.asReportEarlyWriteFailed());
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
      case Action::Type::NewCachedPsk_E:
        this->visitor_(*action.asNewCachedPsk());
        break;
      case Action::Type::SecretAvailable_E:
        this->visitor_(*action.asSecretAvailable());
        break;
    }
  }
}
} // namespace client
} // namespace fizz
