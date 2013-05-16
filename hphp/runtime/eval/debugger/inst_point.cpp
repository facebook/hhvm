/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/eval/debugger/inst_point.h"
#include "hphp/runtime/eval/debugger/debugger.h"
#include "hphp/runtime/eval/debugger/debugger_proxy.h"
#include "hphp/runtime/eval/debugger/debugger_thrift_buffer.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

const uchar* InstPointInfo::lookupPC() {
  TRACE(2, "InstPointInfo::lookupPC\n");
  VMExecutionContext* context = g_vmContext;
  if (m_locType == LocHere) {
    // Instrument to current location
    ActRec *fp = context->getFP();
    if (!fp) {
      return nullptr;
    }
    PC pc = context->getPC();
    HPHP::Unit *unit = fp->m_func->unit();
    if (!unit) {
      return nullptr;
    }
    m_file = unit->filepath()->data();
    m_line = unit->getLineNumber(unit->offsetOf(pc));
    return pc;
  }
  // TODO for file and line
  return nullptr;
}

void InstPointInfo::setLocHere() {
  TRACE(2, "InstPointInfo::setLocHere\n");
  m_locType = LocHere;
}

void InstPointInfo::setLocFileLine(const std::string& file, int line) {
  TRACE(2, "InstPointInfo::setLocFileLine\n");
  m_locType = LocFileLine;
  m_file = file;
  m_line = line;
}

void InstPointInfo::setLocFuncEntry(const std::string& func) {
  TRACE(2, "InstPointInfo::setLocFuncEntry\n");
  m_locType = LocFuncEntry;
  m_func = func;
}

void InstPointInfo::sendImpl(DebuggerThriftBuffer &thrift) {
  TRACE(2, "InstPointInfo::sendImpl\n");
  thrift.write(m_locType);
  thrift.write(m_valid);
  thrift.write(m_file);
  thrift.write(m_line);
  thrift.write(m_func);
  thrift.write(m_desc);
  thrift.write(m_code);
}

void InstPointInfo::recvImpl(DebuggerThriftBuffer &thrift) {
  TRACE(2, "InstPointInfo::recvImpl\n");
  thrift.read(m_locType);
  thrift.read(m_valid);
  thrift.read(m_file);
  thrift.read(m_line);
  thrift.read(m_func);
  thrift.read(m_desc);
  thrift.read(m_code);
}

void InstPointInfo::SendImpl(const InstPointInfoPtrVec& ips,
                             DebuggerThriftBuffer &thrift) {
  TRACE(2, "InstPointInfo::SendImpl\n");
  int16_t size = ips.size();
  thrift.write(size);
  for (int i = 0; i < size; i++) {
    ips[i]->sendImpl(thrift);
  }
}

void InstPointInfo::RecvImpl(InstPointInfoPtrVec& ips,
                             DebuggerThriftBuffer &thrift) {
  TRACE(2, "InstPointInfo::RecvImpl\n");
  int16_t size;
  thrift.read(size);
  ips.resize(size);
  for (int i = 0; i < size; i++) {
    InstPointInfoPtr ipi(new InstPointInfo());
    ipi->recvImpl(thrift);
    ips[i] = ipi;
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
