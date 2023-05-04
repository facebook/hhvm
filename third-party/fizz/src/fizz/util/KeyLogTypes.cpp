#include <fizz/util/KeyLogTypes.h>

namespace fizz {
folly::Optional<NSSLabel> secretToNSSLabel(SecretType secretType) {
  switch (secretType.type()) {
    case SecretType::Type::EarlySecrets_E:
      switch (secretType.tryAsEarlySecrets()) {
        case EarlySecrets::ExternalPskBinder:
          return folly::none;
        case EarlySecrets::ResumptionPskBinder:
          return folly::none;
        case EarlySecrets::ClientEarlyTraffic:
          return NSSLabel::CLIENT_EARLY_TRAFFIC_SECRET;
        case EarlySecrets::EarlyExporter:
          return NSSLabel::EARLY_EXPORTER_SECRET;
        case EarlySecrets::ECHAcceptConfirmation:
          return folly::none;
        case EarlySecrets::HRRECHAcceptConfirmation:
          return folly::none;
      }
    case SecretType::Type::HandshakeSecrets_E:
      switch (secretType.tryAsHandshakeSecrets()) {
        case HandshakeSecrets::ClientHandshakeTraffic:
          return NSSLabel::CLIENT_HANDSHAKE_TRAFFIC_SECRET;
        case HandshakeSecrets::ServerHandshakeTraffic:
          return NSSLabel::SERVER_HANDSHAKE_TRAFFIC_SECRET;
        case HandshakeSecrets::ECHAcceptConfirmation:
          return folly::none;
      }
    case SecretType::Type::MasterSecrets_E:
      switch (secretType.tryAsMasterSecrets()) {
        case MasterSecrets::ExporterMaster:
          return NSSLabel::EXPORTER_SECRET;
        case MasterSecrets::ResumptionMaster:
          return folly::none;
      }
    case SecretType::Type::AppTrafficSecrets_E:
      switch (secretType.tryAsAppTrafficSecrets()) {
        case AppTrafficSecrets::ClientAppTraffic:
          return NSSLabel::CLIENT_TRAFFIC_SECRET_0;
        case AppTrafficSecrets::ServerAppTraffic:
          return NSSLabel::SERVER_TRAFFIC_SECRET_0;
      }
  }
  return folly::none;
}
} // namespace fizz
