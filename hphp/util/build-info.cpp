/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/build-info.h"

#include "hphp/util/embedded-data.h"

#include <folly/Range.h>
#include <atomic>
#include <mutex>
#include <string>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

namespace {

std::atomic<bool> inited;
std::mutex mtx;
std::string repoSchema;
std::string compiler;

////////////////////////////////////////////////////////////////////////////////

/*
 * Initializes the repo schema id and the compiler id from their special
 * sections in the hhvm binary.
 */
void readBuildInfo() {
  if (inited.load(std::memory_order_acquire)) return;
  std::unique_lock<std::mutex> lock(mtx);
  if (inited.load(std::memory_order_acquire)) return;

  auto const get = [&] (const char* section) -> std::string {
    auto constexpr bad = "(UNKNOWN)";

    embedded_data desc;
    if (!get_embedded_data(section, &desc)) {
      return bad;
    }

    auto const data = read_embedded_data(desc);
    if (data.empty()) {
      return bad;
    }

    return data;
  };

  if (auto const env_schema = getenv("HHVM_RUNTIME_REPO_SCHEMA")) {
    repoSchema = env_schema;
  } else {
    repoSchema = get("repo_schema_id");
  }

  compiler = get("compiler_id");

  inited.store(true, std::memory_order_release);
}

}

////////////////////////////////////////////////////////////////////////////////

folly::StringPiece repoSchemaId() {
  readBuildInfo();
  return repoSchema;
}

folly::StringPiece compilerId() {
  readBuildInfo();
  return compiler;
}

////////////////////////////////////////////////////////////////////////////////
}
