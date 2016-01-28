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

#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/boot_timer.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/compression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StaticContentCache StaticContentCache::TheCache;
std::shared_ptr<FileCache> StaticContentCache::TheFileCache;

void StaticContentCache::load() {
  BootTimer::Block timer("loading static content");

  if (!RuntimeOption::FileCache.empty()) {
    TheFileCache = std::make_shared<FileCache>();
    TheFileCache->loadMmap(RuntimeOption::FileCache.c_str());

    Logger::Info("loaded file cache from %s",
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
