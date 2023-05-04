#pragma once
#include <fizz/protocol/KeyScheduler.h>

namespace fizz {
enum class NSSLabel {
  RSA, // 48 bytes for the premaster secret, encoded as 96 hexadecimal
       // characters
  CLIENT_RANDOM, // 48 bytes for the master secret, encoded as 96 hexadecimal
                 // characters (for SSL 3.0, TLS 1.0, 1.1 and 1.2)
  CLIENT_EARLY_TRAFFIC_SECRET, // the hex-encoded early traffic secret for the
                               // client side (for TLS 1.3)
  CLIENT_HANDSHAKE_TRAFFIC_SECRET, // the hex-encoded handshake traffic secret
                                   // for the client side (for TLS 1.3)
  SERVER_HANDSHAKE_TRAFFIC_SECRET, // the hex-encoded handshake traffic secret
                                   // for the server side (for TLS 1.3)
  CLIENT_TRAFFIC_SECRET_0, // the first hex-encoded application traffic secret
                           // for the client side (for TLS 1.3)
  SERVER_TRAFFIC_SECRET_0, // the first hex-encoded application traffic secret
                           // for the server side (for TLS 1.3)
  EARLY_EXPORTER_SECRET, // the hex-encoded early exporter secret (for
                         // TLS 1.3, used for 0-RTT keys in older QUIC
                         // drafts).
  EXPORTER_SECRET // the hex-encoded exporter secret (for TLS 1.3, used for
                  // 1-RTT keys in older QUIC drafts)
};

/**
 * Convert SecretType to NSS Keylog label equivalent.
 * @param secretType The secretType to convert to keylog label.
 * @return the keylog label for secretType
 */
folly::Optional<NSSLabel> secretToNSSLabel(SecretType secretType);

} // namespace fizz
