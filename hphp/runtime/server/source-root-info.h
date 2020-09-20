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

#pragma once

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/debugger/debugger_base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Transport;

struct SourceRootInfo {
  explicit SourceRootInfo(Transport* transport);
  SourceRootInfo(const std::string &user, const std::string &sandbox);
  void createFromUserConfig();
  void createFromCommonRoot(const String &sandboxName);

  ~SourceRootInfo();

  // Set SERVER variables in array, and return the modified version.
  // You'll likely want to ensure the array coming in is a temporary
  // (so it can be modified in place).
  Array setServerVariables(Array server) const;

  std::string path() const;

  bool sandboxOn() const {
    return m_sandboxCond == SandboxCondition::On;
  }

  void clear() {
    m_user.reset();
    m_sandbox.reset();
    m_path.reset();
    m_serverVars.reset();
  }

  bool error() const {
    return m_sandboxCond == SandboxCondition::Error;
  }
  void handleError(Transport *t);
  static const std::string &GetCurrentSourceRoot() {
    return s_path.isNull() ? RuntimeOption::SourceRoot : *s_path;
  }

  static String RelativeToPhpRoot(const String& path) {
    String ret = GetCurrentSourceRoot();
    ret += path;
    return ret;
  }

  Eval::DSandboxInfo getSandboxInfo() const;

private:
  String m_user;
  String m_sandbox;
  String m_path;
  enum class SandboxCondition {
    On,
    Error,
    Off
  } m_sandboxCond;
  Array m_serverVars;
  static THREAD_LOCAL_NO_CHECK(std::string, s_path);

  std::string parseSandboxServerVariable(const std::string &format) const;
};

///////////////////////////////////////////////////////////////////////////////
}

