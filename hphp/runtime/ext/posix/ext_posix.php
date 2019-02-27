<?hh

/**
 * posix_access() checks the user's permission of a file.
 *
 * @param string $file - The name of the file to be tested.
 *
 * @param int $mode - A mask consisting of one or more of POSIX_F_OK,
 *   POSIX_R_OK, POSIX_W_OK and POSIX_X_OK.  POSIX_R_OK, POSIX_W_OK and
 *   POSIX_X_OK request checking whether the file exists and has read, write and
 *   execute permissions, respectively. POSIX_F_OK just requests checking for
 *   the existence of the file.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_access(string $file, int $mode = 0): bool;

/**
 * Generates a string which is the pathname for the current controlling
 *   terminal for the process. On error this will set errno, which can be
 *   checked using posix_get_last_error()
 *
 * @return string - Upon successful completion, returns string of the pathname
 *   to the current controlling terminal. Otherwise FALSE is returned and errno
 *   is set, which can be checked with posix_get_last_error().
 *
 */
<<__Native>>
function posix_ctermid(): string;

/**
 * Retrieve the error number set by the last posix function that failed. The
 *   system error message associated with the errno may be checked with
 *   posix_strerror().
 *
 * @return int - Returns the errno (error number) set by the last posix
 *   function that failed. If no errors exist, 0 is returned.
 *
 */
<<__Native>>
function posix_get_last_error(): int;

/**
 * Retrieve the error number set by the last posix function that failed. The
 *   system error message associated with the errno may be checked with
 *   posix_strerror().
 *
 * @return int - Returns the errno (error number) set by the last posix
 *   function that failed. If no errors exist, 0 is returned.
 *
 */
<<__Native>>
function posix_errno(): int;

/**
 * Gets the absolute pathname of the script's current working directory. On
 *   error, it sets errno which can be checked using posix_get_last_error()
 *
 * @return string - Returns a string of the absolute pathname on success. On
 *   error, returns FALSE and sets errno which can be checked with
 *   posix_get_last_error().
 *
 */
<<__Native>>
function posix_getcwd(): string;

/**
 * Return the numeric effective group ID of the current process.
 *
 * @return int - Returns an integer of the effective group ID.
 *
 */
<<__Native>>
function posix_getegid(): int;

/**
 * Return the numeric effective user ID of the current process. See also
 *   posix_getpwuid() for information on how to convert this into a useable
 *   username.
 *
 * @return int - Returns the user id, as an integer
 *
 */
<<__Native>>
function posix_geteuid(): int;

/**
 * Return the numeric real group ID of the current process.
 *
 * @return int - Returns the real group id, as an integer.
 *
 */
<<__Native>>
function posix_getgid(): int;

/**
 * Gets information about a group provided its id.
 *
 * @param int $gid - The group id.
 *
 * @return mixed - The array elements returned are: The group information
 *   array Element Description name The name element contains the name of the
 *   group. This is a short, usually less than 16 character "handle" of the
 *   group, not the real, full name. passwd The passwd element contains the
 *   group's password in an encrypted format. Often, for example on a system
 *   employing "shadow" passwords, an asterisk is returned instead. gid Group
 *   ID, should be the same as the gid parameter used when calling the function,
 *   and hence redundant. members This consists of an array of string's for all
 *   the members in the group.
 *
 */
<<__Native>>
function posix_getgrgid(int $gid): mixed;

/**
 * Gets information about a group provided its name.
 *
 * @param string $name
 *
 * @return mixed - The array elements returned are: The group information
 *   array Element Description name The name element contains the name of the
 *   group. This is a short, usually less than 16 character "handle" of the
 *   group, not the real, full name. This should be the same as the name
 *   parameter used when calling the function, and hence redundant. passwd The
 *   passwd element contains the group's password in an encrypted format. Often,
 *   for example on a system employing "shadow" passwords, an asterisk is
 *   returned instead. gid Group ID of the group in numeric form. members This
 *   consists of an array of string's for all the members in the group.
 *
 */
<<__Native>>
function posix_getgrnam(string $name): mixed;

/**
 * Gets the group set of the current process.
 *
 * @return mixed - Returns an array of integers containing the numeric group
 *   ids of the group set of the current process.
 *
 */
<<__Native>>
function posix_getgroups(): mixed;

/**
 * Returns the login name of the user owning the current process.
 *
 * @return mixed - Returns the login name of the user, as a string.
 *
 */
<<__Native>>
function posix_getlogin(): mixed;

/**
 * Returns the process group identifier of the process pid or FALSE on
 *   failure.
 *
 * @param int $pid - The process id.
 *
 * @return mixed - Returns the identifier, as an integer.
 *
 */
<<__Native>>
function posix_getpgid(int $pid): mixed;

/**
 * Return the process group identifier of the current process.
 *
 * @return int - Returns the identifier, as an integer.
 *
 */
<<__Native>>
function posix_getpgrp(): int;

/**
 * Return the process identifier of the current process.
 *
 * @return int - Returns the identifier, as an integer.
 *
 */
<<__Native>>
function posix_getpid(): int;

/**
 * Return the process identifier of the parent process of the current process.
 *
 * @return int - Returns the identifier, as an integer.
 *
 */
<<__Native>>
function posix_getppid(): int;

/**
 * Returns an array of information about the given user.
 *
 * @param string $username - An alphanumeric username.
 *
 * @return mixed - On success an array with the following elements is
 *   returned, else FALSE is returned: The user information array Element
 *   Description name The name element contains the username of the user. This
 *   is a short, usually less than 16 character "handle" of the user, not the
 *   real, full name. This should be the same as the username parameter used
 *   when calling the function, and hence redundant. passwd The passwd element
 *   contains the user's password in an encrypted format. Often, for example on
 *   a system employing "shadow" passwords, an asterisk is returned instead. uid
 *   User ID of the user in numeric form. gid The group ID of the user. Use the
 *   function posix_getgrgid() to resolve the group name and a list of its
 *   members. gecos GECOS is an obsolete term that refers to the finger
 *   information field on a Honeywell batch processing system. The field,
 *   however, lives on, and its contents have been formalized by POSIX. The
 *   field contains a comma separated list containing the user's full name,
 *   office phone, office number, and home phone number. On most systems, only
 *   the user's full name is available. dir This element contains the absolute
 *   path to the home directory of the user. shell The shell element contains
 *   the absolute path to the executable of the user's default shell.
 *
 */
<<__Native>>
function posix_getpwnam(string $username): mixed;

/**
 * Returns an array of information about the user referenced by the given user
 *   ID.
 *
 * @param int $uid - The user identifier.
 *
 * @return mixed - Returns an associative array with the following elements:
 *   The user information array Element Description name The name element
 *   contains the username of the user. This is a short, usually less than 16
 *   character "handle" of the user, not the real, full name. passwd The passwd
 *   element contains the user's password in an encrypted format. Often, for
 *   example on a system employing "shadow" passwords, an asterisk is returned
 *   instead. uid User ID, should be the same as the uid parameter used when
 *   calling the function, and hence redundant. gid The group ID of the user.
 *   Use the function posix_getgrgid() to resolve the group name and a list of
 *   its members. gecos GECOS is an obsolete term that refers to the finger
 *   information field on a Honeywell batch processing system. The field,
 *   however, lives on, and its contents have been formalized by POSIX. The
 *   field contains a comma separated list containing the user's full name,
 *   office phone, office number, and home phone number. On most systems, only
 *   the user's full name is available. dir This element contains the absolute
 *   path to the home directory of the user. shell The shell element contains
 *   the absolute path to the executable of the user's default shell.
 *
 */
<<__Native>>
function posix_getpwuid(int $uid): mixed;

/**
 * posix_getrlimit() returns an array of information about the current
 *   resource's soft and hard limits.  Each resource has an associated soft and
 *   hard limit. The soft limit is the value that the kernel enforces for the
 *   corresponding resource. The hard limit acts as a ceiling for the soft
 *   limit. An unprivileged process may only set its soft limit to a value from
 *   0 to the hard limit, and irreversibly lower its hard limit.
 *
 * @return mixed - Returns an associative array of elements for each limit
 *   that is defined. Each limit has a soft and a hard limit. List of possible
 *   limits returned Limit name Limit description core The maximum size of the
 *   core file. When 0, not core files are created. When core files are larger
 *   than this size, they will be truncated at this size. totalmem The maximum
 *   size of the memory of the process, in bytes. virtualmem The maximum size of
 *   the virtual memory for the process, in bytes. data The maximum size of the
 *   data segment for the process, in bytes. stack The maximum size of the
 *   process stack, in bytes. rss The maximum number of virtual pages resident
 *   in RAM maxproc The maximum number of processes that can be created for the
 *   real user ID of the calling process. memlock The maximum number of bytes of
 *   memory that may be locked into RAM. cpu The amount of time the process is
 *   allowed to use the CPU. filesize The maximum size of the data segment for
 *   the process, in bytes. openfiles One more than the maximum number of open
 *   file descriptors.
 *
 */
<<__Native>>
function posix_getrlimit(): mixed;

/**
 * Return the session id of the process pid. The session id of a process is
 *   the process group id of the session leader.
 *
 * @param int $pid - The process identifier. If set to 0, the current process
 *   is assumed. If an invalid pid is specified, then FALSE is returned and an
 *   error is set which can be checked with posix_get_last_error().
 *
 * @return mixed - Returns the identifier, as an integer.
 *
 */
<<__Native>>
function posix_getsid(int $pid): mixed;

/**
 * Return the numeric real user ID of the current process.
 *
 * @return int - Returns the user id, as an integer
 *
 */
<<__Native>>
function posix_getuid(): int;

/**
 * Calculates the group access list for the user specified in name.
 *
 * @param string $name - The user to calculate the list for.
 * @param int $base_group_id - Typically the group number from the password
 *   file.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_initgroups(string $name, int $base_group_id): bool;

/**
 * Determines if the file descriptor fd refers to a valid terminal type
 *   device.
 *
 * @param mixed $fd - The file descriptor.
 *
 * @return bool - Returns TRUE if fd is an open descriptor connected to a
 *   terminal and FALSE otherwise.
 *
 */
<<__Native>>
function posix_isatty(mixed $fd): bool;

/**
 * Send the signal sig to the process with the process identifier pid.
 *
 * @param int $pid - The process identifier.
 * @param int $sig - One of the PCNTL signals constants.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_kill(int $pid, int $sig): bool;

/**
 * posix_mkfifo() creates a special FIFO file which exists in the file system
 *   and acts as a bidirectional communication endpoint for processes.
 *
 * @param string $pathname - Path to the FIFO file.
 *
 * @param int $mode - The second parameter mode has to be given in octal
 *   notation (e.g. 0644). The permission of the newly created FIFO also depends
 *   on the setting of the current umask(). The permissions of the created file
 *   are (mode & ~umask).
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_mkfifo(string $pathname, int $mode): bool;

/**
 * Creates a special or ordinary file.
 *
 * @param string $pathname - The file to create
 *
 * @param int $mode - This parameter is constructed by a bitwise OR between
 *   file type (one of the following constants: POSIX_S_IFREG, POSIX_S_IFCHR,
 *   POSIX_S_IFBLK, POSIX_S_IFIFO or POSIX_S_IFSOCK) and permissions.
 *
 * @param int $major - The major device kernel identifier (required to pass
 *   when using S_IFCHR or S_IFBLK).
 *
 * @param int $minor - The minor device kernel identifier.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_mknod(string $pathname,
                     int $mode,
                     int $major = 0,
                     int $minor = 0): bool;

/**
 * Set the effective group ID of the current process. This is a privileged
 *   function and needs appropriate privileges (usually root) on the system to
 *   be able to perform this function.
 *
 * @param int $gid - The group id.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_setegid(int $gid): bool;

/**
 * Set the real user ID of the current process. This is a privileged function
 *   and needs appropriate privileges (usually root) on the system to be able to
 *   perform this function.
 *
 * @param int $uid - The user id.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_seteuid(int $uid): bool;

/**
 * Set the real group ID of the current process. This is a privileged function
 *   and needs appropriate privileges (usually root) on the system to be able to
 *   perform this function. The appropriate order of function calls is
 *   posix_setgid() first, posix_setuid() last.  If the caller is a super user,
 *   this will also set the effective group id.
 *
 * @param int $gid - The group id.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_setgid(int $gid): bool;

/**
 * Let the process pid join the process group pgid.
 *
 * @param int $pid - The process id.
 * @param int $pgid - The process group id.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_setpgid(int $pid, int $pgid): bool;

/**
 * Make the current process a session leader.
 *
 * @return int - Returns the session id, or -1 on errors.
 *
 */
<<__Native>>
function posix_setsid(): int;

/**
 * Set the real user ID of the current process. This is a privileged function
 *   that needs appropriate privileges (usually root) on the system to be able
 *   to perform this function.
 *
 * @param int $uid - The user id.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function posix_setuid(int $uid): bool;

/**
 * Returns the POSIX system error message associated with the given errno. You
 *   may get the errno parameter by calling posix_get_last_error().
 *
 * @param int $errnum - A POSIX error number, returned by
 *   posix_get_last_error(). If set to 0, then the string "Success" is returned.
 *
 * @return string - Returns the error message, as a string.
 *
 */
<<__Native>>
function posix_strerror(int $errnum): string;

/**
 * Gets information about the current CPU usage.
 *
 * @return mixed - Returns a hash of strings with information about the
 *   current process CPU usage. The indices of the hash are: ticks - the number
 *   of clock ticks that have elapsed since reboot. utime - user time used by
 *   the current process. stime - system time used by the current process.
 *   cutime - user time used by current process and children. cstime - system
 *   time used by current process and children.
 *
 */
<<__Native>>
function posix_times(): mixed;

/**
 * Returns a string for the absolute path to the current terminal device that
 *   is open on the file descriptor fd.
 *
 * @param mixed $fd - The file descriptor.
 *
 * @return mixed - On success, returns a string of the absolute path of the
 *   fd. On failure, returns FALSE
 *
 */
<<__Native>>
function posix_ttyname(mixed $fd): mixed;

/**
 * Gets information about the system.  Posix requires that assumptions must
 *   not be made about the format of the values, e.g. the assumption that a
 *   release may contain three digits or anything else returned by this
 *   function.
 *
 * @return mixed - Returns a hash of strings with information about the
 *   system. The indices of the hash are sysname - operating system name (e.g.
 *   Linux) nodename - system name (e.g. valiant) release - operating system
 *   release (e.g. 2.2.10) version - operating system version (e.g. #4 Tue Jul
 *   20 17:01:36 MEST 1999) machine - system architecture (e.g. i586) domainname
 *   - DNS domainname (e.g. example.com)  domainname is a GNU extension and not
 *   part of POSIX.1, so this field is only available on GNU systems or when
 *   using the GNU libc.
 *
 */
<<__Native>>
function posix_uname(): mixed;
