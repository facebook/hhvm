<?hh

/**
 * This function is identical to the backtick operator.
 *
 * @param string $cmd - The command that will be executed.
 *
 * @return mixed - The output from the executed command.
 *
 */
<<__Native>>
function shell_exec(string $cmd): mixed;

/**
 * exec() executes the given command.
 *
 * @param string $command - The command that will be executed.
 * @param mixed $output - If the output argument is present, then the
 *   specified array will be filled with every line of output from the command.
 *   Trailing whitespace, such as \n, is not included in this array.
 * @param mixed $return_var - If the return_var argument is present along with
 *   the output argument, then the return status of the executed command will be
 *   written to this variable.
 *
 * @return string - The last line from the result of the command. If you need
 *   to execute a command and have all the data from the command passed directly
 *   back without any interference, use the passthru() function.  To get the
 *   output of the executed command, be sure to set and use the output
 *   parameter.
 *
 */
<<__Native>>
function exec(string $command,
              <<__OutOnly("varray")>>
              inout mixed $output,
              <<__OutOnly("KindOfInt64")>>
              inout mixed $return_var): string;

/**
 * The passthru() function is similar to the exec() function in that it
 *   executes a command. This function should be used in place of exec() or
 *   system() when the output from the Unix command is binary data which needs
 *   to be passed directly back to the browser. A common use for this is to
 *   execute something like the pbmplus utilities that can output an image
 *   stream directly. By setting the Content-type to image/gif and then calling
 *   a pbmplus program to output a gif, you can create PHP scripts that output
 *   images directly.
 *
 * @param string $command - The command that will be executed.
 *
 * @param mixed $return_var - If the return_var argument is present, the
 *   return status of the Unix command will be placed here.
 *
 */
<<__Native>>
function passthru(string $command,
                  <<__OutOnly("KindOfInt64")>>
                  inout mixed $return_var): void;

/**
 * system() is just like the C version of the function in that it executes the
 *   given command and outputs the result.  The system() call also tries to
 *   automatically flush the web server's output buffer after each line of
 *   output if PHP is running as a server module.  If you need to execute a
 *   command and have all the data from the command passed directly back without
 *   any interference, use the passthru() function.
 *
 * @param string $command - The command that will be executed.
 * @param mixed $return_var - If the return_var argument is present, then the
 *   return status of the executed command will be written to this variable.
 *
 * @return string - Returns the last line of the command output on success,
 *   and FALSE on failure.
 *
 */
<<__Native>>
function system(string $command,
                <<__OutOnly("KindOfInt64")>>
                inout mixed $return_var): string;

/**
 * proc_open() is similar to popen() but provides a much greater degree of
 *   control over the program execution.
 *
 * @param string $cmd - The command to execute
 * @param array $descriptorspec - An indexed array where the key represents
 *   the descriptor number and the value represents how PHP will pass that
 *   descriptor to the child process. 0 is stdin, 1 is stdout, while 2 is
 *   stderr.  Each element can be: An array describing the pipe to pass to the
 *   process. The first element is the descriptor type and the second element is
 *   an option for the given type. Valid types are pipe (the second element is
 *   either r to pass the read end of the pipe to the process, or w to pass the
 *   write end) and file (the second element is a filename). A stream resource
 *   representing a real file descriptor (e.g. opened file, a socket, STDIN).
 *   The file descriptor numbers are not limited to 0, 1 and 2 - you may specify
 *   any valid file descriptor number and it will be passed to the child
 *   process. This allows your script to interoperate with other scripts that
 *   run as "co-processes". In particular, this is useful for passing
 *   passphrases to programs like PGP, GPG and openssl in a more secure manner.
 *   It is also useful for reading status information provided by those programs
 *   on auxiliary file descriptors.
 * @param mixed $pipes - Will be set to an indexed array of file pointers that
 *   correspond to PHP's end of any pipes that are created.
 * @param string $cwd - The initial working dir for the command. This must be
 *   an absolute directory path, or NULL if you want to use the default value
 *   (the working dir of the current PHP process)
 * @param mixed $env - An array with the environment variables for the command
 *   that will be run, or NULL to use the same environment as the current PHP
 *   process
 * @param mixed $other_options - Allows you to specify additional options.
 *   Currently supported options include: suppress_errors (windows only):
 *   suppresses errors generated by this function when it's set to TRUE
 *   bypass_shell (windows only): bypass cmd.exe shell when set to TRUE context:
 *   stream context used when opening files (created with
 *   stream_context_create()) binary_pipes: open pipes in binary mode, instead
 *   of using the usual stream_encoding
 *
 * @return mixed - Returns a resource representing the process, which should
 *   be freed using proc_close() when you are finished with it. On failure
 *   returns FALSE.
 *
 */
<<__Native>>
function proc_open(string $cmd,
                   darray<int, mixed> $descriptorspec,
                   <<__OutOnly("darray")>>
                   inout mixed $pipes,
                   ?string $cwd = null,
                   mixed $env = null,
                   mixed $other_options = null): mixed;

/**
 * Signals a process (created using proc_open()) that it should terminate.
 *   proc_terminate() returns immediately and does not wait for the process to
 *   terminate.  proc_terminate() allows you terminate the process and continue
 *   with other tasks. You may poll the process (to see if it has stopped yet)
 *   by using the proc_get_status() function.
 *
 * @param resource $process - The proc_open() resource that will be closed.
 * @param int $signal - This optional parameter is only useful on POSIX
 *   operating systems; you may specify a signal to send to the process using
 *   the kill(2) system call. The default is SIGTERM.
 *
 * @return bool - Returns the termination status of the process that was run.
 *
 */
<<__Native>>
function proc_terminate(resource $process, int $signal = 15): bool;

/**
 * proc_close() is similar to pclose() except that it only works on processes
 *   opened by proc_open(). proc_close() waits for the process to terminate, and
 *   returns its exit code. If you have open pipes to that process, you should
 *   fclose() them prior to calling this function in order to avoid a deadlock -
 *   the child process may not be able to exit while the pipes are open.
 *
 * @param resource $process - The proc_open() resource that will be closed.
 *
 * @return int - Returns the termination status of the process that was run.
 *
 */
<<__Native>>
function proc_close(resource $process): int;

/**
 * proc_get_status() fetches data about a process opened using proc_open().
 *
 * @param resource $process - The proc_open() resource that will be evaluated.
 *
 * @return array
 *
 */
<<__Native>>
function proc_get_status(resource $process): darray;

/**
 * proc_nice() changes the priority of the current process by the amount
 *   specified in increment. A positive increment will lower the priority of the
 *   current process, whereas a negative increment will raise the priority.
 *   proc_nice() is not related to proc_open() and its associated functions in
 *   any way.
 *
 * @param int $increment - The increment value of the priority change.
 *
 * @return bool - Returns TRUE on success or FALSE on failure. If an error
 *   occurs, like the user lacks permission to change the priority, an error of
 *   level E_WARNING is also generated.
 *
 */
<<__Native>>
function proc_nice(int $increment): bool;

/**
 * escapeshellarg() adds single quotes around a string and quotes/escapes any
 *   existing single quotes allowing you to pass a string directly to a shell
 *   function and having it be treated as a single safe argument. This function
 *   should be used to escape individual arguments to shell functions coming
 *   from user input. The shell functions include exec(), system() and the
 *   backtick operator.
 *
 * @param string $arg - The argument that will be escaped.
 *
 * @return string - The escaped string.
 *
 */
<<__Native>>
function escapeshellarg(string $arg)[]: string;

/**
 * escapeshellcmd() escapes any characters in a string that might be used to
 *   trick a shell command into executing arbitrary commands. This function
 *   should be used to make sure that any data coming from user input is escaped
 *   before this data is passed to the exec() or system() functions, or to the
 *   backtick operator.  Following characters are preceded by a backslash:
 *   #&;`|*?~<>^()[]{}$\, \x0A and \xFF. ' and " are escaped only if they are
 *   not paired. In Windows, all these characters plus % are replaced by a space
 *   instead.
 *
 * @param string $command - The command that will be escaped.
 *
 * @return string - The escaped string.
 *
 */
<<__Native>>
function escapeshellcmd(string $command): string;
