/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <filesystem>
#include <vector>

#include <folly/Try.h>
#include <folly/futures/Future.h>

#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/runtime/ext/facts/path-and-hash.h"
#include "hphp/runtime/ext/facts/sqlite-key.h"

namespace HPHP {
namespace Facts {

struct Extractor {
  explicit Extractor(folly::Executor& exec) : m_exec(exec) {}

  virtual ~Extractor() = default;

  /**
   * Convert a path/hash tuple representing a file into the JSON-encoded Facts
   * for that file.
   */
  virtual folly::SemiFuture<std::string> get(
      const PathAndOptionalHash& PathAndOptionalHash) = 0;

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

  virtual void prefetchDb(
      const std::filesystem::path& root,
      const SQLiteKey&) = 0;
};

// Call within closed-source code to define a proprietary Extractor.
void setExtractorFactory(ExtractorFactory* factory);

/**
 * Synchronously extract Facts as an encoded blob, from the given absolute path.
 *
 * Throw FactsExtractionExc on error, including the case when a hash
 * is given but doesn't match the loaded file.
 */
std::string facts_binary_from_path(const PathAndOptionalHash& path);

/**
 * Call facts_json_from_path on each path and return the results
 * (or a FactsExtractionExc in the case of an error).
 */
std::vector<folly::Try<FileFacts>> facts_from_paths(
    const std::filesystem::path& root,
    const std::vector<PathAndOptionalHash>& paths);

/**
 * Called by AutoloadMapFactory::getForOptions() to optionally
 * prefetch an autoload db.
 */
void prefetchDb(const std::filesystem::path& root, const SQLiteKey&);

} // namespace Facts
} // namespace HPHP
