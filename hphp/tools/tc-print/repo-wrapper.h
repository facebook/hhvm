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

#ifndef REPO_WRAPPER_
#define REPO_WRAPPER_

#include <map>

#include "hphp/util/sha1.h"
#include "hphp/runtime/vm/repo.h"

namespace HPHP { namespace jit {

struct RepoWrapper {
private:
  typedef std::map<SHA1, Unit*> CacheType;

  Repo* repo;
  CacheType unitCache;

  RepoWrapper(const RepoWrapper& other);
  RepoWrapper& operator=(RepoWrapper other);

public:
  RepoWrapper(const char* repoSchema,
              const std::string& configFile,
              const bool shouldPrint = true);
  ~RepoWrapper();

  void addUnit(Unit* u);
  Unit* getUnit(SHA1 sha1);
};

} }

#endif
