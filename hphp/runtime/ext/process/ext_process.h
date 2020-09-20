/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/extension.h"
#include <sys/wait.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int64_t HHVM_FUNCTION(pcntl_alarm,
                      int seconds);
void HHVM_FUNCTION(pcntl_exec,
                   const String& path,
                   const Array& args = null_array,
                   const Array& envs = null_array);

int64_t HHVM_FUNCTION(pcntl_fork);
Variant HHVM_FUNCTION(pcntl_getpriority,
                      int pid = 0,
                      int process_identifier = 0);
bool HHVM_FUNCTION(pcntl_setpriority,
                   int priority,
                   int pid = 0,
                   int process_identifier = 0);

bool HHVM_FUNCTION(pcntl_signal,
                   int signo,
                   const Variant& handler,
                   bool restart_syscalls = true);
bool HHVM_FUNCTION(pcntl_sigprocmask,
                   int how,
                   const Array& set,
                   Array& oldset);
int64_t HHVM_FUNCTION(pcntl_wait,
                      int64_t& status,
                      int options = 0);
int64_t HHVM_FUNCTION(pcntl_waitpid,
                      int pid,
                      int64_t& status,
                      int options = 0);

int64_t HHVM_FUNCTION(pcntl_wexitstatus,
                      int status);

/**
 * Process pending signals flagged earlier.
 */
bool HHVM_FUNCTION(pcntl_signal_dispatch);

// status querying
bool HHVM_FUNCTION(pcntl_wifexited,
                   int status);
bool HHVM_FUNCTION(pcntl_wifsignaled,
                   int status);
bool HHVM_FUNCTION(pcntl_wifstopped,
                   int status);
int64_t HHVM_FUNCTION(pcntl_wstopsig,
                      int status);
int64_t HHVM_FUNCTION(pcntl_wtermsig,
                      int status);

///////////////////////////////////////////////////////////////////////////////
}

