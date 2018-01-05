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

#ifndef incl_HPHP_VSDEBUG_COMMAND_QUEUE_H_
#define incl_HPHP_VSDEBUG_COMMAND_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <list>

#include "hphp/runtime/ext/vsdebug/command.h"

namespace HPHP {
namespace VSDEBUG {

// This class implements a queue of VS Code Debug Protocol commands to be
// processed by a particular request thread. Commands are enqueued by the
// thread that interacts with the remote debugger client, and are then picked
// up by threads processing PHP requests and executed.
struct CommandQueue {
  CommandQueue();
  virtual ~CommandQueue();

  // Shuts down the command queue and releases any threads that are waiting
  // to process commands.
  void shutdown();

  // Clears any pending messages for this request.
  void clearPendingMessages();

  // Processes debugger commands that are in the queue for this request thread.
  // This call will block until the debugger client orders the target to resume,
  // or the debugger session shuts down.
  void processCommands();

  // Inserts the specified command into the queue for this thread.
  void dispatchCommand(VSCommand* command);

private:

  std::mutex m_lock;

  // Condition that is signaled when a command is available, or the queue
  // is shutting down. A thread waiting in processCommands() should wake.
  std::condition_variable m_wakeWaiterCondition;

  // Condition that is signaled when a waiter has left processCommands(). A
  // thread waiting to confirm the queue is shutdown should wake.
  std::condition_variable m_waiterLeftCondition;

  // Indicates the queue is terminating. Any thread waiting to process commands
  // should unblock and leave.
  std::atomic<bool> m_terminating;

  // Indicates if a thread is currently inside processCommands(). The same
  // thread may enter processCommands multiple times recurisvely when it is
  // evaluating an expression, if the eval hits a breakpoint.
  std::atomic<int> m_threadProcessingCount {0};

  // Queue of commands waiting to be picked up and processed.
  std::list<VSCommand*> m_commands;
};

}
}

#endif // incl_HPHP_VSDEBUG_COMMAND_QUEUE_H_
