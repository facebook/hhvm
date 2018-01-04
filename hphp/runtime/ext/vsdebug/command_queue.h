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

  // Processes debugger commands that are in the queue for this request thread.
  // This call will block until the debugger client orders the target to resume,
  // or the debugger session shuts down.
  // Returns true if any commands were processed, false otherwise.
  bool processCommands();

private:

  std::mutex m_lock;
  std::condition_variable m_condition;
  bool m_terminating;

  // TODO: this is just a placeholder queue for commands. (VS commands aren't
  // implemented yet).
  std::list<int> m_commands;
};

}
}

#endif // incl_HPHP_VSDEBUG_COMMAND_QUEUE_H_
