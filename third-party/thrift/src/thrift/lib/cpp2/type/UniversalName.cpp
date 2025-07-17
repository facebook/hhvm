/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include <thrift/lib/cpp2/type/UniversalName.h>

#include <limits>
#include <string>

#include <openssl/evp.h>

#include <fmt/core.h>
#include <folly/Range.h>
#include <folly/lang/Exception.h>
#include <folly/portability/OpenSSL.h>

namespace apache::thrift::type {

namespace {

const std::string_view kThriftScheme = "fbthrift://";

struct MdCtxDeleter {
  void operator()(EVP_MD_CTX* ctx) const { EVP_MD_CTX_free(ctx); }
};
using ctx_ptr = std::unique_ptr<EVP_MD_CTX, MdCtxDeleter>;

ctx_ptr newMdContext() {
  auto* ctx = EVP_MD_CTX_new();
  if (ctx == nullptr) {
    folly::throw_exception<std::runtime_error>("could not create ctx");
  }
  return ctx_ptr(ctx);
}

void checkResult(int evp_result) {
  if (evp_result == 0) {
    folly::throw_exception<std::runtime_error>("EVP failure");
  }
}

folly::fbstring UniversalHashSha2_256(std::string_view uri) {
  // Save an initalized context.
  static EVP_MD_CTX* kBase = []() {
    auto ctx = newMdContext();
    checkResult(EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr));
    checkResult(EVP_DigestUpdate(
        ctx.get(), kThriftScheme.data(), kThriftScheme.size()));
    return ctx.release(); // Leaky singleton.
  }();

  // Copy the base context.
  auto ctx = newMdContext();
  checkResult(EVP_MD_CTX_copy_ex(ctx.get(), kBase));
  // Digest the uri.
  checkResult(EVP_DigestUpdate(ctx.get(), uri.data(), uri.size()));

  // Get the result.
  folly::fbstring result(EVP_MD_CTX_size(ctx.get()), 0);
  uint32_t size;
  checkResult(EVP_DigestFinal_ex(
      ctx.get(), reinterpret_cast<uint8_t*>(result.data()), &size));
  assert(size == result.size()); // Should already be the correct size.
  result.resize(size);
  return result;
}

hash_size_t UniversalHashSizeSha2_256() {
  return EVP_MD_size(EVP_sha256());
}

} // namespace

folly::fbstring getUniversalHash(
    UniversalHashAlgorithm alg, std::string_view uri) {
  switch (alg) {
    case UniversalHashAlgorithm::Sha2_256:
      return UniversalHashSha2_256(uri);
    default:
      folly::throw_exception<std::runtime_error>(
          "Unsupported type hash algorithm: " + std::to_string((int)alg));
  }
}

hash_size_t getUniversalHashSize(UniversalHashAlgorithm alg) {
  switch (alg) {
    case UniversalHashAlgorithm::Sha2_256:
      return UniversalHashSizeSha2_256();
    default:
      folly::throw_exception<std::runtime_error>(
          "Unsupported type hash algorithm: " + std::to_string((int)alg));
  }
}

void validateUniversalHash(
    UniversalHashAlgorithm alg,
    folly::StringPiece universalHash,
    hash_size_t minHashBytes) {
  auto maxBytes = getUniversalHashSize(alg);
  if (universalHash.size() > std::numeric_limits<hash_size_t>::max()) {
    folly::throw_exception<std::invalid_argument>(fmt::format(
        "Hash size must be <= {}, was {}.", maxBytes, universalHash.size()));
  }
  auto hashBytes = hash_size_t(universalHash.size());
  if (hashBytes < minHashBytes || hashBytes > maxBytes) {
    folly::throw_exception<std::invalid_argument>(fmt::format(
        "Hash size must be in the range [{}, {}], was {}.",
        minHashBytes,
        maxBytes,
        hashBytes));
  }
}

void validateUniversalHashBytes(
    hash_size_t hashBytes, hash_size_t minHashBytes) {
  if (hashBytes == kDisableUniversalHash) {
    return;
  }
  if (hashBytes < minHashBytes) {
    folly::throw_exception<std::invalid_argument>(fmt::format(
        "Hash size must be >= {}, was {}.", minHashBytes, hashBytes));
  }
}

bool matchesUniversalHash(
    folly::StringPiece universalHash, folly::StringPiece prefix) {
  if (universalHash.size() < prefix.size() || prefix.empty()) {
    return false;
  }
  return universalHash.subpiece(0, prefix.size()) == prefix;
}

folly::StringPiece getUniversalHashPrefix(
    folly::StringPiece universalHash, hash_size_t hashBytes) {
  return universalHash.subpiece(0, hashBytes);
}

folly::fbstring maybeGetUniversalHashPrefix(
    UniversalHashAlgorithm alg, std::string_view uri, hash_size_t hashBytes) {
  if (hashBytes == kDisableUniversalHash || // Type hash disabled.
      uri.size() <= size_t(hashBytes)) { // Type uri is smaller.
    return {};
  }
  folly::fbstring result = getUniversalHash(alg, uri);
  if (result.size() > size_t(hashBytes)) {
    result.resize(hashBytes);
  }
  return result;
}

} // namespace apache::thrift::type
