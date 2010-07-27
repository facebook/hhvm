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
  if (m_cached_id.empty() && !m_user.empty()) {
    m_cached_id = m_user + "\t" + m_name + "\t" + m_path;
  }
  return m_cached_id;
}

void SandboxInfo::set(const std::string &id) {
  m_cached_id.clear();
  m_user.clear();
  m_name.clear();
  m_path.clear();
  if (!id.empty()) {
    vector<string> tokens;
    Util::split('\n', id.c_str(), tokens);
    if (tokens.size() == 3) {
      m_user = tokens[0];
      m_name = tokens[1];
      m_path = tokens[2];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
