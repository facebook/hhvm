/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_VSDEBUG_PHP_EXECUTOR_H_
#define incl_HPHP_VSDEBUG_PHP_EXECUTOR_H_

#include "hphp/runtime/ext/vsdebug/command.h"

#include <string>

namespace HPHP {
namespace VSDEBUG {

struct Debugger;
struct DebuggerSession;

struct PHPExecutor {
public:
  virtual ~PHPExecutor();
  PHPExecutor(
    Debugger *debugger,
    DebuggerSession *session,
    const std::string &breakpointFireMessage,
    request_id_t threadId,
    bool evalSilent
  );

  void execute();

protected:
  virtual void callPHPCode() = 0;

  Debugger *m_debugger;
  DebuggerSession *m_session;
  std::string m_breakpointFireMessage;
  request_id_t m_threadId;
  DebuggerRequestInfo* m_ri;
  bool m_evalSilent;
};

}
}

#endif // incl_HPHP_VSDEBUG_HOOK_H_
