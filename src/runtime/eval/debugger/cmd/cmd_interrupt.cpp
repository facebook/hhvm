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

#include <runtime/eval/debugger/cmd/cmd_interrupt.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdInterrupt::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_subtype);
  thrift.write(m_bpi.file);
  thrift.write(m_bpi.line);
  thrift.write(m_exceptionClass);
}

void CmdInterrupt::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_subtype);
  thrift.read(m_bpi.file);
  thrift.read(m_bpi.line);
  thrift.read(m_exceptionClass);
}

bool CmdInterrupt::onClient(DebuggerClient *client) {
  switch (m_subtype) {
    case SessionStarted:
      break;
    case SessionEnded:
      client->info("Program exited normally.");
      break;
    case RequestStarted:
      client->info("Web request started.");
      break;
    case RequestEnded:
      client->info("Web request ended.");
      break;
    case PSPEnded:
      client->info("Post-Send Processing was ended.");
      break;
    case BreakPointReached:
      client->info("Break point reached at %s:%d",
                   m_bpi.file.c_str(), m_bpi.line);
      break;
    case ExceptionThrown:
      client->info("Throwing %s exception at %s:%d",
                   m_exceptionClass.c_str(), m_bpi.file.c_str(), m_bpi.line);
      break;
  }
  return true;
}

bool CmdInterrupt::onServer(DebuggerProxy *proxy) {
  proxy->send(this);
  return true;
}

const char *CmdInterrupt::GetBreakpointName(SubType subtype) {
  switch (subtype) {
    case RequestStarted: return "RequestStarted";
    case RequestEnded:   return "RequestEnded";
    case PSPEnded:       return "PSPEnded";
    default:
      ASSERT(false);
      break;
  }
  return NULL;
}

bool CmdInterrupt::shouldBreak(const BreakPointInfoMap &bps) {
  switch (m_subtype) {
    case SessionStarted:
    case SessionEnded:
      return true; // always break
    case RequestStarted:
    case RequestEnded:
    case PSPEnded:
      return !bps.empty() &&
        bps.find(GetBreakpointName((SubType)m_subtype)) != bps.end();
    case BreakPointReached:
    case ExceptionThrown:
      return !bps.empty() && bps.find(m_bpi.id()) != bps.end();
  }
  ASSERT(false);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
