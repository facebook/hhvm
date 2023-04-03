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

#include "hphp/runtime/server/static-content-cache.h"

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/compression.h"
#include "hphp/util/boot-stats.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/stack-trace.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StaticContentCache StaticContentCache::TheCache;
std::shared_ptr<FileCache> StaticContentCache::TheFileCache;

static StaticString s_file{"file"}, s_line{"line"};

void StaticContentCache::load() {
  BootStats::Block timer("loading static content",
                         RuntimeOption::ServerExecutionMode());

  if (RuntimeOption::RepoAuthoritative && !RuntimeOption::FileCache.empty()) {
    if (StructuredLog::enabled() &&
        StructuredLog::coinflip(RuntimeOption::EvalStaticContentsLogRate)) {
      CacheManager::setLogger([](bool existsCheck, const std::string& name) {
        auto record = StructuredLogEntry{};
        record.setInt("existsCheck", existsCheck);
        record.setStr("file", name);
        bool needsCppStack = true;
        if (!g_context.isNull()) {
          VMRegAnchor _;
          if (vmfp()) {
            auto const bt =
              createBacktrace(BacktraceArgs().withArgValues(false));
            std::vector<std::string> frameStrings;
            std::vector<folly::StringPiece> frames;
            for (int i = 0; i < bt.size(); i++) {
              auto f = tvCastToArrayLike(bt.lookup(i));
              if (f.exists(s_file)) {
                auto s = tvCastToString(f.lookup(s_file)).toCppString();
                if (f.exists(s_line)) {
                  s += folly::sformat(":{}", tvCastToInt64(f.lookup(s_line)));
                }
                frameStrings.emplace_back(std::move(s));
                frames.push_back(frameStrings.back());
              }
            }
            record.setVec("stack", frames);
            needsCppStack = false;
          }
        }
        if (needsCppStack) {
          record.setStackTrace("stack", StackTrace{StackTrace::Force{}});
        }
        StructuredLog::log("hhvm_file_cache", record);
      });
    }

    TheFileCache = std::make_shared<FileCache>();
    TheFileCache->loadMmap(RuntimeOption::FileCache.c_str());

    Logger::Verbose("loaded file cache from %s",
                 RuntimeOption::FileCache.c_str());
    return;
  }
}

bool StaticContentCache::find(const std::string &name, const char *&data,
                              int &len, bool &compressed) const {
  if (TheFileCache) {
    return (data = TheFileCache->read(name.c_str(), len, compressed));
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
