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

#include <util/async_func.h>
#include <runtime/eval/debugger/debugger_thrift_buffer.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(MachineInfo);
class MachineInfo {
public:
  std::string name;
  DebuggerThriftBuffer thrift;
};

///////////////////////////////////////////////////////////////////////////////

class SandboxInfo {
public:
  SandboxInfo() {}
  SandboxInfo(const std::string &id) { set(id);}

  std::string user;
  std::string name;
  std::string path;

  const std::string &id() const;
  void set(const std::string &id);

private:
  mutable std::string cached_id;
};

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(BreakPointInfo);
class BreakPointInfo {
public:
  BreakPointInfo() : line(0) {}

  std::string file;
  int line;
  std::string condition;

  const std::string &id() const;

private:
  mutable std::string cached_id;
};

typedef hphp_string_map<BreakPointInfoPtr> BreakPointInfoMap;

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_EVAL_DEBUGGER_BASE_H__
