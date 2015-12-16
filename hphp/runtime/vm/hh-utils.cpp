/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/hh-utils.h"

#include <atomic>

#include <boost/filesystem.hpp>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

static std::atomic<bool> s_foundHHConfig(false);
void checkHHConfig(const Unit* unit) {

  if (RuntimeOption::RepoAuthoritative ||
      !RuntimeOption::LookForTypechecker ||
      s_foundHHConfig ||
      !unit->isHHFile() ||
      isDebuggerAttached()) {
    return;
  }

  const std::string &s = unit->filepath()->toCppString();
  boost::filesystem::path p(s);

  while (p != "/" && p != "") {
    p.remove_filename();
    p /= ".hhconfig";

    if (boost::filesystem::exists(p)) {
      break;
    }

    p.remove_filename();
  }

  if (p == "/" || p == "") {
    raise_error(
      "%s appears to be a Hack file, but you do not appear to be running "
      "the Hack typechecker. See the documentation at %s for information on "
      "getting it running. You can also set "
      "`-d hhvm.hack.lang.look_for_typechecker=0` "
      "to disable this check (not recommended).",
      s.c_str(),
      "http://docs.hhvm.com/hack/typechecker/setup"
    );
  } else {
    s_foundHHConfig = true;
  }
}

/**
 * The default of "true" here is correct -- see autoTypecheckRequestInit().
 */
static __thread bool tl_doneAutoTypecheck = true;

/**
 * autoTypecheckRequestInit() and autoTypecheckRequestExit() work together to
 * ensure a couple of things.
 *
 * The most obvious is that we only run auto-typechecking once per request;
 * we need to reset the thread-local flag on the next request.
 *
 * More subtle is that we need to block auto-typechecking until the VM is fully
 * initalized, and systemlib is fully merged. In most cases, systemlib is
 * persistent, and we could check SystemLib::s_inited. However, if
 * JitEnableRenameFunction is enabled, then systemlib has to actually be merged
 * every request -- and since much of systemlib is written in Hack, that would
 * trigger auto-typecheck. So we overload the meaning of tl_doneAutoTypecheck to
 * make sure that we don't enable auto-typechecking until systemlib is fully
 * merged.
 */
void autoTypecheckRequestInit() {
  tl_doneAutoTypecheck = false;
}

/**
 * See autoTypecheckRequestInit().
 */
void autoTypecheckRequestExit() {
  tl_doneAutoTypecheck = true;
}

void autoTypecheck(const Unit* unit) {
  if (RuntimeOption::RepoAuthoritative ||
      !RuntimeOption::AutoTypecheck ||
      tl_doneAutoTypecheck ||
      !unit->isHHFile() ||
      isDebuggerAttached()) {
    return;
  }
  tl_doneAutoTypecheck = true;

  vm_call_user_func("\\HH\\Client\\typecheck_and_error",
                    Variant{staticEmptyArray()});
}

}
