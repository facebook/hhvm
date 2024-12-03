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

#include "hphp/runtime/base/configs/repo-loader.h"

#include "hphp/util/build-info.h"
#include "hphp/util/configs/repo.h"

namespace HPHP::Cfg {

void RepoLoader::LocalPathPostProcess(std::string& val) {
  if (val.empty()) {
    const char* path = getenv("HHVM_REPO_LOCAL_PATH");
    if (path != nullptr) {
      val = path;
    }
  }
  replacePlaceholders(val);
}

void RepoLoader::CentralPathPostProcess(std::string& val) {
  if (val.empty()) {
    const char* path = getenv("HHVM_REPO_CENTRAL_PATH");
    if (path != nullptr) {
      val = path;
    }
  }
  replacePlaceholders(val);
}

void RepoLoader::PathPostProcess(std::string& val) {
  if (!val.empty()) {
    replacePlaceholders(val);
    return;
  }
  if (!Cfg::Repo::LocalPath.empty()) {
    val = Cfg::Repo::LocalPath;
    return;
  }
  if (!Cfg::Repo::CentralPath.empty()) {
    val = Cfg::Repo::CentralPath;
    return;
  }
}

}
