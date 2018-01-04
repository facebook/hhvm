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

namespace HPHP {
namespace VSDEBUG {

CommandQueue::CommandQueue() :
  m_terminating(false) {
}

CommandQueue::~CommandQueue() {
  shutdown();
}

void CommandQueue::shutdown() {
  std::lock_guard<std::mutex> lock(m_lock);
  if (!m_terminating) {
    m_terminating = true;
    m_condition.notify_all();
  }
}

bool CommandQueue::processCommands() {
  std::unique_lock<std::mutex> lock(m_lock);
  bool commandProcessed = false;

  while (!m_terminating && !commandProcessed) {
    m_condition.wait(lock);
    if (!m_terminating) {
      for (auto const& command : m_commands) {
        // NOT IMPLEMENTED YET: PROCESS COMMANDS...
        (void)command;
        commandProcessed = true;
      }
    }
  }

  return commandProcessed;
}

}
}
