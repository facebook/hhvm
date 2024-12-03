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

#include "hphp/runtime/ext/facts/config.h"

#include "hphp/util/configs/autoload.h"
#include "hphp/util/user-info.h"

#include <folly/logging/xlog.h>

namespace HPHP::Facts {

::gid_t parseDBGroup() {
  // Resolve the group to a unix gid
  if (Cfg::Autoload::DBGroup.empty()) {
    return -1;
  }
  try {
    GroupInfo grp{Cfg::Autoload::DBGroup.c_str()};
    return grp.gr->gr_gid;
  } catch (const Exception& e) {
    XLOGF(
        WARN,
        "Can't resolve {} to a gid: {}",
        Cfg::Autoload::DBGroup,
        e.what());
    return -1;
  }
}

::mode_t parseDBPerms() {
  try {
    ::mode_t res = std::stoi(Cfg::Autoload::DBPerms, 0, 8);
    XLOGF(DBG0, "Converted {} to {:04o}", Cfg::Autoload::DBPerms, res);
    return res;
  } catch (const std::exception& e) {
    XLOG(WARN) << "Error running std::stoi on \"Autoload.DB.Perms\": "
               << e.what();
    return 0644;
  }
}

} // namespace HPHP::Facts
