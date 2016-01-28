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

#ifndef REPO_WRAPPER_
#define REPO_WRAPPER_

#include <map>

#include "hphp/util/md5.h"
#include "hphp/runtime/vm/repo.h"

namespace HPHP { namespace jit {

class RepoWrapper {
  typedef std::map<MD5, Unit*> CacheType;

  Repo* repo;
  CacheType unitCache;

  RepoWrapper(const RepoWrapper& other);
  RepoWrapper& operator=(RepoWrapper other);

public:
  RepoWrapper(const char* repoSchema, const std::string& configFile);
  ~RepoWrapper();

  void addUnit(Unit* u);
  Unit* getUnit(MD5 md5);
};

} }

#endif
