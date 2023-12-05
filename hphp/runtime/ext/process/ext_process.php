<?hh

/**
 * Creates a timer that will send a SIGALRM signal to the process after the
 *   given number of seconds. Any call to pcntl_alarm() will cancel any
 *   previously set alarm.
 *
 * @param int $seconds - The number of seconds to wait. If seconds is zero, no
 *   new alarm is created.
 *
 * @return int - Returns the time in seconds that any previously scheduled
 *   alarm had remaining before it was to be delivered, or 0 if there was no
 *   previously scheduled alarm.
 *
 */
<<__Native>>
function pcntl_alarm(int $seconds): int;

/**
 * Executes the program with the given arguments.
 *
 * @param string $path - path must be the path to a binary executable or a
 *   script with a valid path pointing to an executable in the shebang (
 *   #!/usr/local/bin/perl for example) as the first line. See your system's man
 *   execve(2) page for additional information.
 * @param array $args - args is an array of argument strings passed to the
 *   program.
 *
 * @param array $envs - envs is an array of strings which are passed as
 *   environment to the program. The array is in the format of name => value,
 *   the key being the name of the environmental variable and the value being
 *   the value of that variable.
 *
 */
<<__Native>>
function pcntl_exec(
  string $path,
  varray<string> $args = vec[],
  darray<string, string> $envs = dict[],
): void;

/**
 * The pcntl_fork() function creates a child process that differs from the
 *   parent process only in its PID and PPID. Please see your system's fork(2)
 *   man page for specific details as to how fork works on your system.
 *
 * @return int - On success, the PID of the child process is returned in the
 *   parent's thread of execution, and a 0 is returned in the child's thread of
 *   execution. On failure, a -1 will be returned in the parent's context, no
 *   child process will be created, and a PHP error is raised.
 *
 */
<<__Native>>
function pcntl_fork(): int;

/**
 * pcntl_getpriority() gets the priority of pid. Because priority levels can
 *   differ between system types and kernel versions, please see your system's
 *   getpriority(2) man page for specific details.
 *
 * @param int $pid - If not specified, the pid of the current process is used.
 * @param int $process_identifier - One of PRIO_PGRP, PRIO_USER or
 *   PRIO_PROCESS.
 *
 * @return mixed - pcntl_getpriority() returns the priority of the process or
 *   FALSE on error. A lower numerical value causes more favorable scheduling.
 *   WarningThis function may return Boolean FALSE, but may also return a
 *   non-Boolean value which evaluates to FALSE, such as 0 or "". Please read
 *   the section on Booleans for more information. Use the === operator for
 *   testing the return value of this function.
 *
 */
<<__Native>>
function pcntl_getpriority(int $pid = 0, int $process_identifier = 0): mixed;

/**
 * pcntl_setpriority() sets the priority of pid.
 *
 * @param int $priority - priority is generally a value in the range -20 to
 *   20. The default priority is 0 while a lower numerical value causes more
 *   favorable scheduling. Because priority levels can differ between system
 *   types and kernel versions, please see your system's setpriority(2) man page
 *   for specific details.
 * @param int $pid - If not specified, the pid of the current process is used.
 * @param int $process_identifier - One of PRIO_PGRP, PRIO_USER or
 *   PRIO_PROCESS.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function pcntl_setpriority(int $priority,
                           int $pid = 0,
                           int $process_identifier = 0): bool;

/**
 * The pcntl_signal() function installs a new signal handler for the signal
 *   indicated by signo.
 *
 * @param int $signo - The signal number.
 * @param mixed $handler - The signal handler which may be the name of a user
 *   created function, or method, or either of the two global constants SIG_IGN
 *   or SIG_DFL.  Note that when you set a handler to an object method, that
 *   object's reference count is increased which makes it persist until you
 *   either change the handler to something else, or your script ends.
 * @param bool $restart_syscalls - Specifies whether system call restarting
 *   should be used when this signal arrives.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function pcntl_signal(int $signo,
                      mixed $handler,
                      bool $restart_syscalls = true): bool;

/**
 * The pcntl_sigprocmask() function adds, removes or sets blocked signals,
 * depending on the how parameter.
 *
 * @param int $how - Sets the behavior of pcntl_sigprocmask(). Possible values:
 *  - SIG_BLOCK: Add the signals to the currently blocked signals.
 *  - SIG_UNBLOCK: Remove the signals from the currently blocked signals.
 *  - SIG_SETMASK: Replace the currently blocked signals by the given list of
 *    signals.
 * @param array $set - List of signals.
 * @param array& $oldset - The oldset parameter is set to an array containing
 *   the list of the previously blocked signals.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function pcntl_sigprocmask(
  int $how,
  varray<int> $set,
  <<__OutOnly("varray")>>
  inout mixed $oldset
): bool;

/**
 * The wait function suspends execution of the current process until a child
 *   has exited, or until a signal is delivered whose action is to terminate the
 *   current process or to call a signal handling function. If a child has
 *   already exited by the time of the call (a so-called "zombie" process), the
 *   function returns immediately. Any system resources used by the child are
 *   freed. Please see your system's wait(2) man page for specific details as to
 *   how wait works on your system.  This function is equivalent to calling
 *   pcntl_waitpid() with a -1 pid and no options.
 *
 * @param mixed $status - pcntl_wait() will store status information in the
 *   status parameter which can be evaluated using the following functions:
 *   pcntl_wifexited(), pcntl_wifstopped(), pcntl_wifsignaled(),
 *   pcntl_wexitstatus(), pcntl_wtermsig() and pcntl_wstopsig().
 * @param int $options - If wait3 is available on your system (mostly
 *   BSD-style systems), you can provide the optional options parameter. If this
 *   parameter is not provided, wait will be used for the system call. If wait3
 *   is not available, providing a value for options will have no effect. The
 *   value of options is the value of zero or more of the following two
 *   constants OR'ed together: Possible values for options WNOHANG Return
 *   immediately if no child has exited. WUNTRACED Return for children which are
 *   stopped, and whose status has not been reported.
 *
 * @return int - pcntl_wait() returns the process ID of the child which
 *   exited, -1 on error or zero if WNOHANG was provided as an option (on
 *   wait3-available systems) and no child was available.
 *
 */
<<__Native>>
function pcntl_wait(
  <<__OutOnly("KindOfInt64")>>
  inout mixed $status,
  int $options = 0
): int;

/**
 * Suspends execution of the current process until a child as specified by the
 *   pid argument has exited, or until a signal is delivered whose action is to
 *   terminate the current process or to call a signal handling function.  If a
 *   child as requested by pid has already exited by the time of the call (a
 *   so-called "zombie" process), the function returns immediately. Any system
 *   resources used by the child are freed. Please see your system's waitpid(2)
 *   man page for specific details as to how waitpid works on your system.
 *
 * @param int $pid - The value of pid can be one of the following: possible
 *   values for pid < -1 wait for any child process whose process group ID is
 *   equal to the absolute value of pid. -1 wait for any child process; this is
 *   the same behaviour that the wait function exhibits. 0 wait for any child
 *   process whose process group ID is equal to that of the calling process. > 0
 *   wait for the child whose process ID is equal to the value of pid.
 *   Specifying -1 as the pid is equivalent to the functionality pcntl_wait()
 *   provides (minus options).
 * @param mixed $status - pcntl_waitpid() will store status information in the
 *   status parameter which can be evaluated using the following functions:
 *   pcntl_wifexited(), pcntl_wifstopped(), pcntl_wifsignaled(),
 *   pcntl_wexitstatus(), pcntl_wtermsig() and pcntl_wstopsig().
 * @param int $options - The value of options is the value of zero or more of
 *   the following two global constants OR'ed together: possible values for
 *   options WNOHANG return immediately if no child has exited. WUNTRACED return
 *   for children which are stopped, and whose status has not been reported.
 *
 * @return int - pcntl_waitpid() returns the process ID of the child which
 *   exited, -1 on error or zero if WNOHANG was used and no child was available
 *
 */
<<__Native>>
function pcntl_waitpid(int $pid,
                       <<__OutOnly("KindOfInt64")>>
                       inout mixed $status,
                       int $options = 0): int;

/**
 * Returns the return code of a terminated child. This function is only useful
 *   if pcntl_wifexited() returned TRUE.
 *
 * @param int $status - status parameter is the status parameter supplied to a
 *   successful call to pcntl_waitpid().
 *
 * @return int - Returns the return code, as an integer.
 *
 */
<<__Native>>
function pcntl_wexitstatus(int $status): int;

/**
 * Checks whether the child status code represents a normal exit.
 *
 * @param int $status - status parameter is the status parameter supplied to a
 *   successful call to pcntl_waitpid().
 *
 * @return bool - Returns TRUE if the child status code represents a normal
 *   exit, FALSE otherwise.
 *
 */
<<__Native>>
function pcntl_wifexited(int $status): bool;

/**
 * Checks whether the child process exited because of a signal which was not
 *   caught.
 *
 * @param int $status - status parameter is the status parameter supplied to a
 *   successful call to pcntl_waitpid().
 *
 * @return bool - Returns TRUE if the child process exited because of a signal
 *   which was not caught, FALSE otherwise.
 *
 */
<<__Native>>
function pcntl_wifsignaled(int $status): bool;

/**
 * Checks whether the child process which caused the return is currently
 *   stopped; this is only possible if the call to pcntl_waitpid() was done
 *   using the option WUNTRACED.
 *
 * @param int $status - status parameter is the status parameter supplied to a
 *   successful call to pcntl_waitpid().
 *
 * @return bool - Returns TRUE if the child process which caused the return is
 *   currently stopped, FALSE otherwise.
 *
 */
<<__Native>>
function pcntl_wifstopped(int $status): bool;

/**
 * Returns the number of the signal which caused the child to stop. This
 *   function is only useful if pcntl_wifstopped() returned TRUE.
 *
 * @param int $status - status parameter is the status parameter supplied to a
 *   successful call to pcntl_waitpid().
 *
 * @return int - Returns the signal number.
 *
 */
<<__Native>>
function pcntl_wstopsig(int $status): int;

/**
 * Returns the number of the signal that caused the child process to
 *   terminate. This function is only useful if pcntl_wifsignaled() returned
 *   TRUE.
 *
 * @param int $status - status parameter is the status parameter supplied to a
 *   successful call to pcntl_waitpid().
 *
 * @return int - Returns the signal number, as an integer.
 *
 */
<<__Native>>
function pcntl_wtermsig(int $status): int;

/**
 * The pcntl_signal_dispatch() function calls the signal handlers installed by
 *   pcntl_signal() for each pending signal.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function pcntl_signal_dispatch(): bool;
