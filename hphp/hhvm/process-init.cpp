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

#include "hphp/runtime/vm/builtin-symbol-map.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/build-info.h"
#include "hphp/util/logger.h"

#include <folly/portability/Libgen.h>

#include <string>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

void ProcessInit() {
  if (RuntimeOption::RepoAuthoritative) {
    RepoFile::loadGlobalTables();
    RepoFile::globalData().load();
  }
  Stack::ValidateStackSize();
}

///////////////////////////////////////////////////////////////////////////////
}
