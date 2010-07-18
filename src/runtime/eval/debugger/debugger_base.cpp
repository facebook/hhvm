/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/debugger/debugger_base.h>
#include <util/util.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

const std::string &SandboxInfo::id() const {
  if (cached_id.empty() && !user.empty()) {
    cached_id = user + "\t" + name + "\t" + path;
  }
  return cached_id;
}

void SandboxInfo::set(const std::string &id) {
  cached_id.clear();
  user.clear();
  name.clear();
  path.clear();
  if (!id.empty()) {
    vector<string> tokens;
    Util::split('\n', id.c_str(), tokens);
    if (tokens.size() == 3) {
      user = tokens[0];
      name = tokens[1];
      path = tokens[2];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

const std::string &BreakPointInfo::id() const {
  if (cached_id.empty()) {
    cached_id = file + ":" + boost::lexical_cast<string>(line);
  }
  return cached_id;
}

///////////////////////////////////////////////////////////////////////////////
}}
