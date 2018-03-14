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

#include "hphp/runtime/ext/vsdebug/command_queue.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"

namespace HPHP {
namespace VSDEBUG {

CommandQueue::CommandQueue() :
  m_terminating(false) {
}

CommandQueue::~CommandQueue() {
  shutdown();
}

void CommandQueue::shutdown() {
  std::unique_lock<std::mutex> lock(m_lock);
  if (!m_terminating) {
    m_terminating = true;
    m_wakeWaiterCondition.notify_all();

    // If a thread is currently in processCommands(), wait for it to
    // exit before returning.
    while (m_threadProcessingCount.load() > 0) {
      m_waiterLeftCondition.wait(lock);
    }
    assertx(m_threadProcessingCount.load() == 0);
  }

  // Free any commands remaining in the queue.
  for (auto it = m_commands.begin(); it != m_commands.end();) {
    delete *it;
    it = m_commands.erase(it);
  }

  assertx(m_commands.empty());
}

void CommandQueue::processCommands() {
  std::unique_lock<std::mutex> lock(m_lock);

  if (m_terminating) {
    return;
  }

  m_threadProcessingCount++;

  SCOPE_EXIT {
    assertx(lock.owns_lock());
    m_threadProcessingCount--;

    if (m_terminating) {
      // Let the thread that is waiting for us in shutdown() proceed.
      m_waiterLeftCondition.notify_all();
    }
  };

  while (!m_terminating) {
    while (m_commands.empty()) {
      m_wakeWaiterCondition.wait(lock);
      if (m_terminating) {
        break;
      }
    }

    while (!m_commands.empty()) {
      if (m_terminating) {
        break;
      }

      auto command = m_commands.front();
      m_commands.pop_front();

      // We must drop the lock before executing the command so that the
      // client thread can enqueue additional commands while we process.
      // Additionally, this is going to call back into Debugger and needs to
      // be able to acquire the debugger lock.
      lock.unlock();

      bool resumeTarget = command->execute();

      // Free the command.
      delete command;

      // Re-acquire the lock before proceeding.
      lock.lock();

      if (resumeTarget) {
        // The command indicated we should resume the target. Release the
        // current thread from this routine.
        return;
      }
    }
  }
}

void CommandQueue::clearPendingMessages() {
  std::unique_lock<std::mutex> lock(m_lock);
  m_commands.clear();
}

void CommandQueue::dispatchCommand(VSCommand* command) {
  std::unique_lock<std::mutex> lock(m_lock);
  m_commands.push_back(command);
  m_wakeWaiterCondition.notify_all();
}

}
}
