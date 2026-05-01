/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

namespace fizz {
namespace openssl {

template <typename AeadCipher>
std::unique_ptr<Aead> OpenSSLEVPCipher::makeCipher() {
  static_assert(AeadCipher::kIVLength >= sizeof(uint64_t), "iv too small");
  static_assert(AeadCipher::kIVLength < kMaxIVLength, "iv too large");
  static_assert(AeadCipher::kTagLength < kMaxTagLength, "tag too large");
  std::unique_ptr<OpenSSLEVPCipher> cipher;
  Error err;
  FIZZ_THROW_ON_ERROR(
      OpenSSLEVPCipher::create(
          cipher,
          err,
          AeadCipher::kKeyLength,
          AeadCipher::kIVLength,
          AeadCipher::kTagLength,
          Properties<AeadCipher>::Cipher(),
          Properties<AeadCipher>::kOperatesInBlocks,
          Properties<AeadCipher>::kRequiresPresetTagLen),
      err);
  return std::unique_ptr<Aead>(std::move(cipher));
}

} // namespace openssl
} // namespace fizz
