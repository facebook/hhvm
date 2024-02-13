// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <filesystem>
#include <vector>

#include <folly/Try.h>
#include <folly/futures/Future.h>

#include "hphp/runtime/ext/facts/path-and-hash.h"

namespace HPHP {
namespace Decl {

struct DeclBinaryString {
  std::string value;
};

struct Extractor {
  explicit Extractor(folly::Executor& exec) : m_exec(exec) {}

  virtual ~Extractor() = default;

  /**
   * Convert a path/hash tuple representing a file into the JSON-encoded Facts
   * for that file.
   */
  virtual folly::SemiFuture<std::string> get(
      const Facts::PathAndOptionalHash& pathAndOptionalHash) = 0;

 protected:
  Extractor() = delete;
  Extractor(const Extractor&) = delete;
  Extractor(Extractor&&) noexcept = delete;
  Extractor& operator=(const Extractor&) = delete;
  Extractor& operator=(Extractor&&) noexcept = delete;

  folly::Executor& m_exec;
};

struct ExtractorFactory {
  virtual ~ExtractorFactory() = default;

  virtual std::unique_ptr<Extractor> make(folly::Executor&) = 0;
};

// Call within closed-source code to define a proprietary Extractor.
void setExtractorFactory(ExtractorFactory* factory);

/*
 * Synchronously extract Decls as an encoded blob, from the given absolute path.
 *
 * Throw DeclExtractionExc on error.
 */
DeclBinaryString decls_binary_from_path(const Facts::PathAndOptionalHash& path);

} // namespace Decl
} // namespace HPHP
