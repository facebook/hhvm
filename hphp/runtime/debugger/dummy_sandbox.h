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

#ifndef incl_HPHP_EVAL_DUMMY_SANDBOX_H_
#define incl_HPHP_EVAL_DUMMY_SANDBOX_H_

#include "hphp/util/async-func.h"
#include "hphp/util/synchronizable.h"
#include "hphp/runtime/debugger/debugger_base.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

/**
 * Serves as execution thread when remote debugger is not attached to a web
 * request.
 */
class DebuggerProxy;
DECLARE_BOOST_TYPES(DummySandbox);
class DummySandbox : public Synchronizable {
public:
  DummySandbox(DebuggerProxy *proxy, const std::string &defaultPath,
               const std::string &startupFile);
  void start();
  bool stop(int timeout);

  // execution thread
  void run();

  void notifySignal(int signum);

private:
  DebuggerProxy *m_proxy;
  std::string m_defaultPath;
  std::string m_startupFile;

  AsyncFunc<DummySandbox> m_thread;
  bool m_stopped;
  int m_signum;

  std::string getStartupDoc(const DSandboxInfo &sandbox);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DUMMY_SANDBOX_H_
