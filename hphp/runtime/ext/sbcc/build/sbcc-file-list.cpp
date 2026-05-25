/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext/sbcc/build/sbcc-file-list.h"

#include <sys/stat.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_set>

#include <folly/String.h>
#include <fmt/core.h>

#include "hphp/util/logger.h"

namespace HPHP {

std::vector<SBCCSourceFile> loadSBCCSourceFiles(
    const std::string& sourceRoot,
    const std::string& fileListPath,
    bool skipMissing) {
  std::ifstream in(fileListPath);
  if (!in) {
    throw std::runtime_error(
      fmt::format("Cannot open file list: {}", fileListPath));
  }

  std::unordered_set<std::string> seen;
  std::vector<SBCCSourceFile> files;
  std::string line;
  size_t skipped = 0;

  while (std::getline(in, line)) {
    if (line.empty()) continue;
    if (!seen.insert(line).second) continue; // duplicate

    // Reject absolute paths — entries must be repo-relative under sourceRoot.
    if (!line.empty() && line[0] == '/') {
      throw std::runtime_error(fmt::format(
        "File list contains absolute path: '{}'. "
        "Entries must be repo-relative (e.g. 'flib/foo.php', not '/tmp/foo.php').",
        line));
    }

    SBCCSourceFile entry;
    entry.relativePath = line;
    entry.absPath = sourceRoot + line;

    struct stat st;
    if (stat(entry.absPath.c_str(), &st) == 0) {
      entry.fileSize = st.st_size;
    } else if (skipMissing) {
      skipped++;
      continue;
    } else {
      throw std::runtime_error(
        fmt::format("Cannot stat file: {}", entry.absPath));
    }

    files.push_back(std::move(entry));
  }

  if (skipped > 0) {
    Logger::FInfo("sbcc-file-list: skipped {} missing files", skipped);
  }

  // Sort largest-first for better worker load balancing.
  std::sort(files.begin(), files.end(),
            [](const SBCCSourceFile& a, const SBCCSourceFile& b) {
              return a.fileSize > b.fileSize;
            });

  Logger::FInfo("sbcc-file-list: loaded {} files from {}",
                files.size(), fileListPath);
  return files;
}

} // namespace HPHP
