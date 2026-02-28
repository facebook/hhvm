/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include <cstdint>
#include <pthread.h>
#include <signal.h>

namespace HPHP {

using sighandler_sync_t = void(*)(int);
using sigaction_sync_t = void(*)(int, siginfo_t*);

// Similar to signal(), but allows non-async-signal-safe functions to be used in
// handlers. Return false upon failure.
bool sync_signal(int signo, sighandler_sync_t h = nullptr);

// Synchronous signal handling for sigaction() style handlers.
bool sync_signal_info(int signo, sigaction_sync_t h = nullptr);

// Block signals that we plan to handle synchronously, and start the thread to
// process the signals.
void block_sync_signals_and_start_handler_thread();

// Unblock sync signals and handle them using the default handlers, instead of
// in the thread.
void reset_sync_signals();

// Unblock sync signals and ignore them (instead of handing them with the
// default hander).
void ignore_sync_signals();

// Whether a signal is intended to be forwarded to PHP code.
bool is_sync_signal(int signo);

// Restart the signal handler thread in the child process after fork.  Only used
// when the child process will continue running HHVM, e.g., pcntl_fork().
void postfork_restart_handler_thread();

}

