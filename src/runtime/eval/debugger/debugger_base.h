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

#ifndef __HPHP_EVAL_DEBUGGER_BASE_H__
#define __HPHP_EVAL_DEBUGGER_BASE_H__

#include <runtime/eval/debugger/break_point.h>
#include <runtime/base/util/exceptions.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class DebuggerClientExitException : public Exception {};
class DebuggerConsoleExitException : public Exception {};

class DebuggerRestartException : public Exception {
public:
  DebuggerRestartException(StringVecPtr args) : m_args(args) {}
  StringVecPtr m_args;
};

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(MachineInfo);
class MachineInfo {
public:
  std::string m_name;
  DebuggerThriftBuffer m_thrift;
};

///////////////////////////////////////////////////////////////////////////////

class SandboxInfo {
public:
  SandboxInfo() {}
  SandboxInfo(const std::string &id) { set(id);}

  std::string m_user;
  std::string m_name;
  std::string m_path;

  const std::string &id() const;
  void set(const std::string &id);

private:
  mutable std::string m_cached_id;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_BASE_H__
