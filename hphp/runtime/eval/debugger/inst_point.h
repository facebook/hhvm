/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EVAL_DEBUGGER_INST_POINT_H_
#define incl_HPHP_EVAL_DEBUGGER_INST_POINT_H_

#include <runtime/base/complex_types.h>
#include <runtime/eval/debugger/debugger_thrift_buffer.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(InstPointInfo);
class InstPointInfo {
public:
  InstPointInfo() : m_locType(LocTypeCount), m_valid(false), m_line(0) {}

  const uchar* lookupPC();

  // Set loc based on file and line, called on client side
  void setLocHere();
  void setLocFileLine(const std::string& file, int line);
  void setLocFuncEntry(const std::string& func);

  void sendImpl(DebuggerThriftBuffer &thrift);
  void recvImpl(DebuggerThriftBuffer &thrift);

  static void SendImpl(const InstPointInfoPtrVec& ips,
                       DebuggerThriftBuffer &thrift);
  static void RecvImpl(InstPointInfoPtrVec& ips,
                       DebuggerThriftBuffer &thrift);

public:
  enum LocationType {
    LocHere,
    LocFileLine,
    LocFuncEntry,
    LocTypeCount
  };
  int m_locType;
  bool m_valid;
  std::string m_file;
  int32_t m_line;
  std::string m_func;
  // TODO: add more info

  std::string m_desc;
  std::string m_code;
};


///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_INST_POINT_H_
