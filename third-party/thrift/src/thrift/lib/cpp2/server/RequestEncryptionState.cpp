// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "thrift/lib/cpp2/server/RequestEncryptionState.h"

namespace apache::thrift {

folly::StringPiece toString(RequestEncryptionState state) {
  switch (state) {
    case RequestEncryptionState::Plaintext:
      return "plaintext";
    case RequestEncryptionState::Encrypted:
      return "encrypted";
    case RequestEncryptionState::StoptlsEncrypted:
      return "stoptls_encrypted";
    case RequestEncryptionState::StoptlsSkipped:
      return "stoptls_skipped";
  }
  return "plaintext"; // unreachable; satisfies non-exhaustive switch warning
}

} // namespace apache::thrift
