/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

namespace fizz {

template <typename EVPImpl>
std::unique_ptr<Aead> OpenSSLEVPCipher::makeCipher() {
  static_assert(EVPImpl::kIVLength >= sizeof(uint64_t), "iv too small");
  static_assert(EVPImpl::kIVLength < kMaxIVLength, "iv too large");
  static_assert(EVPImpl::kTagLength < kMaxTagLength, "tag too large");
  return std::unique_ptr<Aead>(new OpenSSLEVPCipher(
      EVPImpl::kKeyLength,
      EVPImpl::kIVLength,
      EVPImpl::kTagLength,
      EVPImpl::Cipher(),
      EVPImpl::kOperatesInBlocks,
      EVPImpl::kRequiresPresetTagLen));
}

} // namespace fizz
