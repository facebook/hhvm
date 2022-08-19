/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/mysqld_daemon.h"

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "sql/sql_const.h"
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

namespace {
bool is_daemon_proc = false;
}

/**
  Prediacate to test if we're currently executing
  in the daemon process.
  @retval true if this is the daemon
  @retval false otherwise
 */
bool mysqld::runtime::is_daemon() { return is_daemon_proc; }

/**
  Daemonize mysqld.

  This function does sysv style of daemonization of mysqld.

  @retval In daemon; file descriptor for the write end of the status pipe.
  @retval In parent; -1 if successful.
  @retval In parent; -2 in case of errors.
*/
int mysqld::runtime::mysqld_daemonize() {
  int pipe_fd[2];
  if (pipe(pipe_fd) < 0) return -2;

  pid_t pid = fork();
  if (pid == -1) {
    // Error
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    return -2;
  }

  if (pid != 0) {
    // Parent, close write end of pipe.
    close(pipe_fd[1]);

    // Wait for first child to fork successfully.
    int rc, status;
    char waitstatus;
    while ((rc = waitpid(pid, &status, 0)) == -1 && errno == EINTR) {
      // Retry if errno is EINTR.
    }
    if (rc == -1) {
      LogErr(ERROR_LEVEL, ER_WAITPID_FAILED, static_cast<long long>(pid));
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      return -2;
    }
    // The error log is now owned by the daemon, and anything buffered
    // up will be dumped by it. So we just discard the buffered messages here.
    discard_error_log_messages();

    // Parent waits for pipe message from grand child
    rc = read(pipe_fd[0], &waitstatus, 1);
    close(pipe_fd[0]);

    if (rc != 1) {
      LogErr(ERROR_LEVEL, ER_FAILED_TO_FIND_MYSQLD_STATUS, strerror(errno), rc);
      return -2;
    } else if (waitstatus != 1) {
      return -2;
    }
    // Parent should return to calling function (mysqld_main) and not
    // call exit() directly.
    return -1;
  } else {
    // Child, close read end of pipe file descriptor.
    close(pipe_fd[0]);

    int stdinfd;
    if ((stdinfd = open("/dev/null", O_RDONLY)) <= STDERR_FILENO) {
      close(pipe_fd[1]);
      exit(MYSQLD_ABORT_EXIT);
    }

    if (!(dup2(stdinfd, STDIN_FILENO) != STDIN_FILENO) && (setsid() > -1)) {
      close(stdinfd);
      pid_t grand_child_pid = fork();
      switch (grand_child_pid) {
        case 0:  // Grand child
          is_daemon_proc = true;
          return pipe_fd[1];
        case -1:
          close(pipe_fd[1]);
          _exit(MYSQLD_FAILURE_EXIT);
        default:
          _exit(MYSQLD_SUCCESS_EXIT);
      }
    } else {
      close(stdinfd);
      close(pipe_fd[1]);
      _exit(MYSQLD_SUCCESS_EXIT);
    }
  }
}

/**
  Signal parent to exit.

  @param pipe_write_fd File Descriptor of write end of pipe.

  @param status status of the initialization done by grand child.
                1 means initialization complete and the server
                  is ready to accept client connections.
                0 means intialization aborted due to some failure.

  @note This function writes the status to write end of pipe.
  This notifies the parent which is block on read end of pipe.
*/
void mysqld::runtime::signal_parent(int pipe_write_fd, char status) {
  if (pipe_write_fd != -1) {
    while (write(pipe_write_fd, &status, 1) == -1 && errno == EINTR) {
      // Retry write syscall if errno is EINTR.
    }

    close(pipe_write_fd);
  }
}
