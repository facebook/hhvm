<?hh

namespace {

const int FILE_TEXT = 0;
const int FILE_BINARY = 0;

/**
 * fopen() binds a named resource, specified by filename, to a stream.
 *
 * @return mixed - Returns a file pointer resource on success, or FALSE on
 *   error.
 *
 */
<<__Native>>
function fopen(string $filename,
               string $mode,
               bool $use_include_path = false,
               mixed $context = null): mixed;

/**
 * Opens a pipe to a process executed by forking the command given by command.
 *
 * @param string $command - The command
 * @param string $mode - The mode
 *
 * @return mixed - Returns a file pointer identical to that returned by
 *   fopen(), except that it is unidirectional (may only be used for reading or
 *   writing) and must be closed with pclose(). This pointer may be used with
 *   fgets(), fgetss(), and fwrite().  If an error occurs, returns FALSE.
 *
 */
<<__Native>>
function popen(string $command, string $mode): mixed;

/**
 * The file pointed to by handle is closed.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen() or fsockopen().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function fclose(resource $handle): bool;

/**
 * Closes a file pointer to a pipe opened by popen().
 *
 * @param resource $handle - The file pointer must be valid, and must have
 *   been returned by a successful call to popen().
 *
 * @return mixed - Returns the termination status of the process that was run.
 *
 */
<<__Native>>
function pclose(mixed $handle): mixed;

/**
 * Sets the file position indicator for the file referenced by handle. The new
 *   position, measured in bytes from the beginning of the file, is obtained by
 *   adding offset to the position specified by whence.
 *
 * @param resource $handle - resource that is typically created using fopen().
 * @param int $offset - The offset.  To move to a position before the
 *   end-of-file, you need to pass a negative value in offset and set whence to
 *   SEEK_END.
 * @param int $whence - whence values are: SEEK_SET - Set position equal to
 *   offset bytes. SEEK_CUR - Set position to current location plus offset.
 *   SEEK_END - Set position to end-of-file plus offset.
 *
 * @return mixed - Upon success, returns 0; otherwise, returns -1. Note that
 *   seeking past EOF is not considered an error.
 *
 */
<<__Native>>
function fseek(resource $handle, int $offset, int $whence = SEEK_SET): mixed;

/**
 * Sets the file position indicator for handle to the beginning of the file
 *   stream.  If you have opened the file in append ("a" or "a+") mode, any data
 *   you write to the file will always be appended, regardless of the file
 *   position.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function rewind(resource $handle): bool;

/**
 * Returns the position of the file pointer referenced by handle.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen() or popen(). ftell() gives undefined
 *   results for append-only streams (opened with "a" flag).
 *
 * @return mixed - Returns the position of the file pointer referenced by
 *   handle as an integer; i.e., its offset into the file stream.  If an error
 *   occurs, returns FALSE.
 *
 */
<<__Native>>
function ftell(resource $handle): mixed;

/**
 * Tests for end-of-file on a file pointer.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen() or fsockopen() (and not yet closed by
 *   fclose()).
 *
 * @return bool - Returns TRUE if the file pointer is at EOF or an error
 *   occurs (including socket timeout); otherwise returns FALSE.
 *
 */
<<__Native>>
function feof(resource $handle): bool;

/**
 * Gathers the statistics of the file opened by the file pointer handle. This
 *   function is similar to the stat() function except that it operates on an
 *   open file pointer instead of a filename.
 *
 * @param resource $handle - resource that is typically created using fopen().
 *
 * @return mixed - Returns an array with the statistics of the file; the
 *   format of the array is described in detail on the stat() manual page.
 *
 */
<<__Native>>
function fstat(resource $handle): mixed;

/**
 * fread() reads up to length bytes from the file pointer referenced by
 *   handle. Reading stops as soon as one of the following conditions is met:
 *   length bytes have been read EOF (end of file) is reached a packet becomes
 *   available (for network streams) 8192 bytes have been read (after opening
 *   userspace stream)
 *
 * @param resource $handle - resource that is typically created using fopen().
 * @param int $length - Up to length number of bytes read.
 *
 * @return mixed - Returns the read string or FALSE on failure.
 *
 */
<<__Native>>
function fread(resource $handle, int $length): mixed;

/**
 * Gets a character from the given file pointer.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen() or fsockopen() (and not yet closed by
 *   fclose()).
 *
 * @return mixed - Returns a string containing a single character read from
 *   the file pointed to by handle. Returns FALSE on EOF. WarningThis function
 *   may return Boolean FALSE, but may also return a non-Boolean value which
 *   evaluates to FALSE, such as 0 or "". Please read the section on Booleans
 *   for more information. Use the === operator for testing the return value of
 *   this function.
 *
 */
<<__Native>>
function fgetc(resource $handle): mixed;

/**
 * Gets a line from file pointer.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen() or fsockopen() (and not yet closed by
 *   fclose()).
 *
 * @param int $length - Reading ends when length - 1 bytes have been read, on
 *   a newline (which is included in the return value), or on EOF (whichever
 *   comes first). If no length is specified, it will keep reading from the
 *   stream until it reaches the end of the line.  Until PHP 4.3.0, omitting it
 *   would assume 1024 as the line length. If the majority of the lines in the
 *   file are all larger than 8KB, it is more resource efficient for your script
 *   to specify the maximum line length.
 *
 * @return mixed - Returns a string of up to length - 1 bytes read from the
 *   file pointed to by handle.  If an error occurs, returns FALSE.
 *
 */
<<__Native>>
function fgets(resource $handle, int $length = 0): mixed;

/**
 * Identical to fgets(), except that fgetss() attempts to strip any NUL bytes,
 *   HTML and PHP tags from the text it reads.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen() or fsockopen() (and not yet closed by
 *   fclose()).
 * @param int $length - Length of the data to be retrieved.
 * @param string $allowable_tags - You can use the optional third parameter to
 *   specify tags which should not be stripped.
 *
 * @return mixed - Returns a string of up to length - 1 bytes read from the
 *   file pointed to by handle, with all HTML and PHP code stripped.  If an
 *   error occurs, returns FALSE.
 *
 */
<<__Native>>
function fgetss(resource $handle,
                int $length = 0,
                string $allowable_tags = ""): mixed;

/**
 * The function fscanf() is similar to sscanf(), but it takes its input from a
 *   file associated with handle and interprets the input according to the
 *   specified format, which is described in the documentation for sprintf().
 *   Any whitespace in the format string matches any whitespace in the input
 *   stream. This means that even a tab \t in the format string can match a
 *   single space character in the input stream.  Each call to fscanf() reads
 *   one line from the file.
 *
 * @param resource $handle - resource that is typically created using fopen().
 * @param string $format - The specified format as described in the sprintf()
 *   documentation.
 *
 * @return mixed - false on EOF, otherwise varray of parsed values
 *
 */
<<__Native>>
function fscanf(resource $handle, string $format): mixed;

/**
 * Reads to EOF on the given file pointer from the current position and writes
 *   the results to the output buffer.  You may need to call rewind() to reset
 *   the file pointer to the beginning of the file if you have already written
 *   data to the file.  If you just want to dump the contents of a file to the
 *   output buffer, without first modifying it or seeking to a particular
 *   offset, you may want to use the readfile(), which saves you the fopen()
 *   call.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen() or fsockopen() (and not yet closed by
 *   fclose()).
 *
 * @return mixed - If an error occurs, fpassthru() returns FALSE. Otherwise,
 *   fpassthru() returns the number of characters read from handle and passed
 *   through to the output.
 *
 */
<<__Native>>
function fpassthru(resource $handle): mixed;

/**
 * @param resource $handle - resource that is typically created using fopen().
 * @param string $data - The string that is to be written.
 * @param int $length - If the length argument is given, writing will stop
 *   after length bytes have been written or the end of string is reached,
 *   whichever comes first.  Note that if the length argument is given, then the
 *   magic_quotes_runtime configuration option will be ignored and no slashes
 *   will be stripped from string.
 *
 * @return mixed - fwrite() returns the number of bytes written, or FALSE on
 *   error.
 *
 */
<<__Native>>
function fwrite(resource $handle, string $data, int $length = 0): mixed;

<<__Native>>
function fputs(resource $handle, string $data, int $length = 0): mixed;

/**
 * Write a string produced according to format to the stream resource
 *   specified by handle.
 *
 * @param resource $handle - resource that is typically created using fopen().
 * @param string $format - See sprintf() for a description of format.
 *
 * @return mixed - Returns the length of the string written.
 *
 */
<<__Native>>
function fprintf(mixed $handle, string $format, mixed... $argv): mixed;
// TODO(T121572869): This function probably ought to take a format string
// rather than a normal string.

/**
 * Write a string produced according to format to the stream resource
 *   specified by handle.  Operates as fprintf() but accepts an array of
 *   arguments, rather than a variable number of arguments.
 *
 * @param resource $handle
 * @param string $format - See sprintf() for a description of format.
 * @param array $args
 *
 * @return mixed - Returns the length of the outputted string.
 *
 */
<<__Native>>
function vfprintf(mixed $handle, mixed $format, mixed $args): mixed;

/**
 * This function forces a write of all buffered output to the resource pointed
 *   to by the file handle.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen() or fsockopen() (and not yet closed by
 *   fclose()).
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function fflush(resource $handle): bool;

/**
 * Takes the filepointer, handle, and truncates the file to length, size.
 *
 * @param resource $handle - The file pointer.  The handle must be open for
 *   writing.
 * @param int $size - The size to truncate to.  If size is larger than the
 *   file then the file is extended with null bytes.  If size is smaller than
 *   the file then the file is truncated to that size.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ftruncate(resource $handle, int $size): bool;

/**
 * flock() allows you to perform a simple reader/writer model which can be
 *   used on virtually every platform (including most Unix derivatives and even
 *   Windows).  On versions of PHP before 5.3.2, the lock is released also by
 *   fclose() (which is also called automatically when script finished).  PHP
 *   supports a portable way of locking complete files in an advisory way (which
 *   means all accessing programs have to use the same way of locking or it will
 *   not work). By default, this function will block until the requested lock is
 *   acquired; this may be controlled (on non-Windows platforms) with the
 *   LOCK_NB option documented below.
 *
 * @param resource $handle - resource that is typically created using fopen().
 * @param int $operation - operation is one of the following: LOCK_SH to
 *   acquire a shared lock (reader). LOCK_EX to acquire an exclusive lock
 *   (writer). LOCK_UN to release a lock (shared or exclusive).  It is also
 *   possible to add LOCK_NB as a bitmask to one of the above operations if you
 *   don't want flock() to block while locking. (not supported on Windows)
 * @param mixed $wouldblock - The optional third argument is set to TRUE if
 *   the lock would block (EWOULDBLOCK errno condition). (not supported on
 *   Windows)
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function flock(resource $handle,
               int $operation,
               <<__OutOnly("KindOfBoolean")>>
               inout mixed $wouldblock): bool;

/**
 * fputcsv() formats a line (passed as a fields array) as CSV and write it
 *   (terminated by a newline) to the specified file handle.
 *
 * @param resource $handle - The file pointer must be valid, and must point to
 *   a file successfully opened by fopen() or fsockopen() (and not yet closed by
 *   fclose()).
 * @param array $fields - An array of values.
 * @param string $delimiter - The optional delimiter parameter sets the field
 *   delimiter (one character only).
 * @param string $enclosure - The optional enclosure parameter sets the field
 *   enclosure (one character only).
 * @param string $escape_char - Set the escape character (one character only).
 *
 * @return mixed - Returns the length of the written string or FALSE on
 *   failure.
 *
 */
<<__Native>>
function fputcsv(resource $handle,
                 varray<string> $fields,
                 string $delimiter = ",",
                 string $enclosure = "\"",
                 string $escape_char = "\\"): mixed;

/**
 * Similar to fgets() except that fgetcsv() parses the line it reads for
 *   fields in CSV format and returns an array containing the fields read.
 *
 * @param resource $handle - A valid file pointer to a file successfully
 *   opened by fopen(), popen(), or fsockopen().
 * @param int $length - Must be greater than the longest line (in characters)
 *   to be found in the CSV file (allowing for trailing line-end characters). It
 *   became optional in PHP 5. Omitting this parameter (or setting it to 0 in
 *   PHP 5.0.4 and later) the maximum line length is not limited, which is
 *   slightly slower.
 * @param string $delimiter - Set the field delimiter (one character only).
 * @param string $enclosure - Set the field enclosure character (one character
 *   only).
 * @param string $escape - Set the escape character (one character only).
 *
 * @return mixed - Returns an indexed array containing the fields read.  A
 *   blank line in a CSV file will be returned as an array comprising a single
 *   null field, and will not be treated as an error. If PHP is not properly
 *   recognizing the line endings when reading files either on or created by a
 *   Macintosh computer, enabling the auto_detect_line_endings run-time
 *   configuration option may help resolve the problem.  fgetcsv() returns NULL
 *   if an invalid handle is supplied or FALSE on other errors, including end of
 *   file.
 *
 */
<<__Native>>
function fgetcsv(resource $handle,
                 int $length = 0,
                 string $delimiter = ",",
                 string $enclosure = "\"",
                 string $escape = "\\"): mixed;

/**
 * This function is similar to file(), except that file_get_contents() returns
 *   the file in a string, starting at the specified offset up to maxlen bytes.
 *   On failure, file_get_contents() will return FALSE.  file_get_contents() is
 *   the preferred way to read the contents of a file into a string. It will use
 *   memory mapping techniques if supported by your OS to enhance performance.
 *   If you're opening a URI with special characters, such as spaces, you need
 *   to encode the URI with urlencode().  The default value of maxlen is not
 *   actually -1; rather, it is an internal PHP value which means to copy the
 *   entire stream until end-of-file is reached. The only way to specify this
 *   default value is to leave it out of the parameter list.
 *
 * @param string $filename - Name of the file to read.
 * @param bool $use_include_path - As of PHP 5 the FILE_USE_INCLUDE_PATH can
 *   be used to trigger include path search.
 * @param mixed $context - A valid context resource created with
 *   stream_context_create(). If you don't need to use a custom context, you can
 *   skip this parameter by NULL.
 * @param int $offset - The offset where the reading starts on the original
 *   stream.
 * @param int $maxlen - Maximum length of data read. The default is to read
 *   until end of file is reached. Note that this parameter is applied to the
 *   stream processed by the filters.
 *
 * @return mixed - The function returns the read data or FALSE on failure.
 *
 */
<<__Native>>
function file_get_contents(string $filename,
                           bool $use_include_path = false,
                           mixed $context = null,
                           int $offset = -1,
                           int $maxlen = -1): mixed;

/**
 * This function is identical to calling fopen(), fwrite() and fclose()
 *   successively to write data to a file.  If filename does not exist, the file
 *   is created. Otherwise, the existing file is overwritten, unless the
 *   FILE_APPEND flags is set.
 *
 * @param string $filename - Path to the file where to write the data.
 * @param mixed $data - The data to write. Can be either a string, an array or
 *   a stream resource.  If data is a stream resource, the remaining buffer of
 *   that stream will be copied to the specified file. This is similar with
 *   using stream_copy_to_stream().  You can also specify the data parameter as
 *   a single dimension array. This is equivalent to
 *   file_put_contents($filename, implode('', $array)).
 * @param int $flags - The value of flags can be any combination of the
 *   following flags (with some restrictions), joined with the binary OR (|)
 *   operator.  Available flags Flag Description FILE_USE_INCLUDE_PATH Search
 *   for filename in the include directory. See include_path for more
 *   information. FILE_APPEND If file filename already exists, append the data
 *   to the file instead of overwriting it. Mutually exclusive with LOCK_EX
 *   since appends are atomic and thus there is no reason to lock. LOCK_EX
 *   Acquire an exclusive lock on the file while proceeding to the writing.
 *   Mutually exclusive with FILE_APPEND.
 * @param mixed $context - A valid context resource created with
 *   stream_context_create().
 *
 * @return mixed - The function returns the number of bytes that were written
 *   to the file, or FALSE on failure.
 *
 */
<<__Native>>
function file_put_contents(string $filename,
                           mixed $data,
                           int $flags = 0,
                           mixed $context = null): mixed;

/**
 * Reads an entire file into an array.  You can use file_get_contents() to
 *   return the contents of a file as a string.
 *
 * @param string $filename - Path to the file. TipA URL can be used as a
 *   filename with this function if the fopen wrappers have been enabled. See
 *   fopen() for more details on how to specify the filename. See the List of
 *   Supported Protocols/Wrappers for links to information about what abilities
 *   the various wrappers have, notes on their usage, and information on any
 *   predefined variables they may provide.
 * @param int $flags - The optional parameter flags can be one, or more, of
 *   the following constants: FILE_USE_INCLUDE_PATH Search for the file in the
 *   include_path.
 * @param mixed $context - Do not add newline at the end of each array element
 *
 * @return mixed - Returns the file in an array. Each element of the array
 *   corresponds to a line in the file, with the newline still attached. Upon
 *   failure, file() returns FALSE.  Each line in the resulting array will
 *   include the line ending, unless FILE_IGNORE_NEW_LINES is used, so you still
 *   need to use rtrim() if you do not want the line ending present. If PHP is
 *   not properly recognizing the line endings when reading files either on or
 *   created by a Macintosh computer, enabling the auto_detect_line_endings
 *   run-time configuration option may help resolve the problem.
 *
 */
<<__Native>>
function file(string $filename, int $flags = 0, mixed $context = null): mixed;

/**
 * Reads a file and writes it to the output buffer.
 *
 * @param string $filename - The filename being read.
 * @param bool $use_include_path - You can use the optional second parameter
 *   and set it to TRUE, if you want to search for the file in the include_path,
 *   too.
 * @param mixed $context - A context stream resource.
 *
 * @return mixed - Returns the number of bytes read from the file. If an error
 *   occurs, FALSE is returned and unless the function was called as
 *   @readfile(), an error message is printed.
 *
 */
<<__Native>>
function readfile(string $filename,
                  bool $use_include_path = false,
                  mixed $context = null): mixed;

/**
 * This function checks to ensure that the file designated by filename is a
 *   valid upload file (meaning that it was uploaded via PHP's HTTP POST upload
 *   mechanism). If the file is valid, it will be moved to the filename given by
 *   destination.  This sort of check is especially important if there is any
 *   chance that anything done with uploaded files could reveal their contents
 *   to the user, or even to other users on the same system.
 *
 * @param string $filename - The filename of the uploaded file.
 * @param string $destination - The destination of the moved file.
 *
 * @return bool - If filename is not a valid upload file, then no action will
 *   occur, and move_uploaded_file() will return FALSE.  If filename is a valid
 *   upload file, but cannot be moved for some reason, no action will occur, and
 *   move_uploaded_file() will return FALSE. Additionally, a warning will be
 *   issued.
 *
 */
<<__Native>>
function move_uploaded_file(string $filename, string $destination): bool;

/**
 * parse_ini_file() loads in the ini file specified in filename, and returns
 *   the settings in it in an associative array.  The structure of the ini file
 *   is the same as the php.ini's.
 *
 * @param string $filename - The filename of the ini file being parsed.
 * @param bool $process_sections - By setting the process_sections parameter
 *   to TRUE, you get a multidimensional array, with the section names and
 *   settings included. The default for process_sections is FALSE
 * @param int $scanner_mode - Can either be INI_SCANNER_NORMAL (default) or
 *   INI_SCANNER_RAW. If INI_SCANNER_RAW is supplied, then option values will
 *   not be parsed.
 *
 * @return mixed - The settings are returned as an associative array on
 *   success, and FALSE on failure.
 *
 */
<<__Native>>
function parse_ini_file(string $filename,
                        bool $process_sections = false,
                        int $scanner_mode = INI_SCANNER_NORMAL): mixed;

/**
 * parse_ini_string() returns the settings in string ini in an associative
 *   array.  The structure of the ini string is the same as the php.ini's.
 *
 * @param string $ini - The contents of the ini file being parsed.
 * @param bool $process_sections - By setting the process_sections parameter
 *   to TRUE, you get a multidimensional array, with the section names and
 *   settings included. The default for process_sections is FALSE
 * @param int $scanner_mode - Can either be INI_SCANNER_NORMAL (default) or
 *   INI_SCANNER_RAW. If INI_SCANNER_RAW is supplied, then option values will
 *   not be parsed.
 *
 * @return mixed - The settings are returned as an associative array on
 *   success, and FALSE on failure.
 *
 */
<<__Native>>
function parse_ini_string(string $ini,
                          bool $process_sections = false,
                          int $scanner_mode = INI_SCANNER_NORMAL): mixed;

/**
 * Calculates the MD5 hash of the file specified by the filename parameter
 *   using the RSA Data Security, Inc. MD5 Message-Digest Algorithm, and returns
 *   that hash. The hash is a 32-character hexadecimal number.
 *
 * @param string $filename - The filename
 * @param bool $raw_output - When TRUE, returns the digest in raw binary
 *   format with a length of 16.
 *
 * @return mixed - Returns a string on success, FALSE otherwise.
 *
 */
<<__Native>>
function md5_file(string $filename, bool $raw_output = false): mixed;

/**
 * @param string $filename - The filename of the file to hash.
 * @param bool $raw_output - When TRUE, returns the digest in raw binary
 *   format with a length of 20.
 *
 * @return mixed - Returns a string on success, FALSE otherwise.
 *
 */
<<__Native>>
function sha1_file(string $filename, bool $raw_output = false): mixed;

/**
 * Attempts to change the mode of the specified file to that given in mode.
 *
 * @param string $filename - Path to the file.
 * @param int $mode - Note that mode is not automatically assumed to be an
 *   octal value, so strings (such as "g+w") will not work properly. To ensure
 *   the expected operation, you need to prefix mode with a zero (0):      The
 *   mode parameter consists of three octal number components specifying access
 *   restrictions for the owner, the user group in which the owner is in, and to
 *   everybody else in this order. One component can be computed by adding up
 *   the needed permissions for that target user base. Number 1 means that you
 *   grant execute rights, number 2 means that you make the file writable,
 *   number 4 means that you make the file readable. Add up these numbers to
 *   specify needed rights. You can also read more about modes on Unix systems
 *   with 'man 1 chmod' and 'man 2 chmod'.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function chmod(string $filename, int $mode): bool;

/**
 * Attempts to change the owner of the file filename to user user. Only the
 *   superuser may change the owner of a file.
 *
 * @param string $filename - Path to the file.
 * @param mixed $user - A user name or number.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function chown(string $filename, mixed $user): bool;

/**
 * Attempts to change the owner of the symlink filename to user user.  Only
 *   the superuser may change the owner of a symlink.
 *
 * @param string $filename - Path to the file.
 * @param mixed $user - User name or number.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function lchown(string $filename, mixed $user): bool;

/**
 * Attempts to change the group of the file filename to group.  Only the
 *   superuser may change the group of a file arbitrarily; other users may
 *   change the group of a file to any group of which that user is a member.
 *
 * @param string $filename - Path to the file.
 * @param mixed $group - A group name or number.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function chgrp(string $filename, mixed $group): bool;

/**
 * Attempts to change the group of the symlink filename to group.  Only the
 *   superuser may change the group of a symlink arbitrarily; other users may
 *   change the group of a symlink to any group of which that user is a member.
 *
 * @param string $filename - Path to the symlink.
 * @param mixed $group - The group specified by name or number.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function lchgrp(string $filename, mixed $group): bool;

/**
 * Attempts to set the access and modification times of the file named in the
 *   filename parameter to the value given in time. Note that the access time is
 *   always modified, regardless of the number of parameters.  If the file does
 *   not exist, it will be created.
 *
 * @param string $filename - The name of the file being touched.
 * @param int $mtime - The touch time. If time is not supplied, the current
 *   system time is used.
 * @param int $atime - If present, the access time of the given filename is
 *   set to the value of atime. Otherwise, it is set to time.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function touch(string $filename, int $mtime = 0, int $atime = 0): bool;

/**
 * Makes a copy of the file source to dest.  If you wish to move a file, use
 *   the rename() function.
 *
 * @param string $source - Path to the source file.
 * @param string $dest - The destination path. If dest is a URL, the copy
 *   operation may fail if the wrapper does not support overwriting of existing
 *   files. Warning  If the destination file already exists, it will be
 *   overwritten.
 * @param mixed $context - A valid context resource created with
 *   stream_context_create().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function copy(string $source, string $dest, mixed $context = null): bool;

/**
 * Attempts to rename oldname to newname.
 *
 * @param string $oldname - The old name. The wrapper used in oldname must
 *   match the wrapper used in newname.
 * @param string $newname - The new name.
 * @param mixed $context - Context support was added with PHP 5.0.0. For a
 *   description of contexts, refer to Stream Functions.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function rename(string $oldname, string $newname, mixed $context = null): bool;

/**
 * umask() sets PHP's umask to mask & 0777 and returns the old umask. When PHP
 *   is being used as a server module, the umask is restored when each request
 *   is finished.
 *
 * @param mixed $mask - The new umask.
 *
 * @return int - umask() without arguments simply returns the current umask
 *   otherwise the old umask is returned.
 *
 */
<<__Native>>
function umask(mixed $mask = null): int;

/**
 * Deletes filename. Similar to the Unix C unlink() function. A E_WARNING
 *   level error will be generated on failure.
 *
 * @param string $filename - Path to the file.
 * @param mixed $context - Context support was added with PHP 5.0.0. For a
 *   description of contexts, refer to Stream Functions.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function unlink(string $filename, mixed $context = null): bool;

/**
 * link() creates a hard link.
 *
 * @param string $target - The link name.
 * @param string $link - Target of the link.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function link(string $target, string $link): bool;

/**
 * symlink() creates a symbolic link to the existing target with the specified
 *   name link.
 *
 * @param string $target - Target of the link.
 * @param string $link - The link name.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function symlink(string $target, string $link): bool;

/**
 * Given a string containing a path to a file, this function will return the
 *   base name of the file.
 *
 * @param string $path - A path.  On Windows, both slash (/) and backslash (\)
 *   are used as directory separator character. In other environments, it is the
 *   forward slash (/).
 * @param string $suffix - If the filename ends in suffix this will also be
 *   cut off.
 *
 * @return string - Returns the base name of the given path.
 *
 */
<<__Native, __IsFoldable>>
function basename(string $path, string $suffix = "")[]: string;

/**
 * fnmatch() checks if the passed string would match the given shell wildcard
 *   pattern.
 *
 * @param string $pattern - The shell wildcard pattern.
 * @param string $filename - The tested string. This function is especially
 *   useful for filenames, but may also be used on regular strings.  The average
 *   user may be used to shell patterns or at least in their simplest form to
 *   '?' and '*' wildcards so using fnmatch() instead of preg_match() for
 *   frontend search expression input may be way more convenient for
 *   non-programming users.
 * @param int $flags - The value of flags can be any combination of the
 *   following flags, joined with the binary OR (|) operator. A list of possible
 *   flags for fnmatch() Flag Description FNM_NOESCAPE Disable backslash
 *   escaping. FNM_PATHNAME Slash in string only matches slash in the given
 *   pattern. FNM_PERIOD Leading period in string must be exactly matched by
 *   period in the given pattern. FNM_CASEFOLD Caseless match. Part of the GNU
 *   extension.
 *
 * @return bool - Returns TRUE if there is a match, FALSE otherwise.
 *
 */
<<__Native>>
function fnmatch(string $pattern, string $filename, int $flags = 0): bool;

/**
 * The glob() function searches for all the pathnames matching pattern
 *   according to the rules used by the libc glob() function, which is similar
 *   to the rules used by common shells.
 *
 * @param string $pattern - The pattern. No tilde expansion or parameter
 *   substitution is done.
 * @param int $flags - Valid flags: GLOB_MARK - Adds a slash to each directory
 *   returned GLOB_NOSORT - Return files as they appear in the directory (no
 *   sorting) GLOB_NOCHECK - Return the search pattern if no files matching it
 *   were found GLOB_NOESCAPE - Backslashes do not quote metacharacters
 *   GLOB_BRACE - Expands {a,b,c} to match 'a', 'b', or 'c' GLOB_ONLYDIR -
 *   Return only directory entries which match the pattern GLOB_ERR - Stop on
 *   read errors (like unreadable directories), by default errors are ignored.
 *
 * @return mixed - Returns an array containing the matched files/directories,
 *   an empty array if no file matched or FALSE on error.  On some systems it is
 *   impossible to distinguish between empty match and an error.
 *
 */
<<__Native>>
function glob(string $pattern, int $flags = 0): mixed;

/**
 * Creates a file with a unique filename, with access permission set to 0600,
 *   in the specified directory. If the directory does not exist, tempnam() may
 *   generate a file in the system's temporary directory, and return the name of
 *   that.
 *
 * @param string $dir - The directory where the temporary filename will be
 *   created.
 * @param string $prefix - The prefix of the generated temporary filename.
 *   Windows uses only the first three characters of prefix.
 *
 * @return mixed - Returns the new temporary filename, or FALSE on failure.
 *
 */
<<__Native>>
function tempnam(string $dir, string $prefix): mixed;

/**
 * Creates a temporary file with a unique name in read-write (w+) mode and
 *   returns a file handle .  The file is automatically removed when closed
 *   (using fclose()), or when the script ends.  For details, consult your
 *   system documentation on the tmpfile(3) function, as well as the stdio.h
 *   header file.
 *
 * @return mixed - Returns a file handle, similar to the one returned by
 *   fopen(), for the new file or FALSE on failure.
 *
 */
<<__Native>>
function tmpfile(): mixed;

/**
 * Gets permissions for the given file.
 *
 * @param string $filename - Path to the file.
 *
 * @return mixed - Returns the permissions on the file, or FALSE on failure.
 *
 */
<<__Native>>
function fileperms(string $filename): mixed;

/**
 * Gets the file inode.
 *
 * @param string $filename - Path to the file.
 *
 * @return mixed - Returns the inode number of the file, or FALSE on failure.
 *
 */
<<__Native>>
function fileinode(string $filename): mixed;

/**
 * Gets the size for the given file.
 *
 * @param string $filename - Path to the file.
 *
 * @return mixed - Returns the size of the file in bytes, or FALSE (and
 *   generates an error of level E_WARNING) in case of an error. Because PHP's
 *   integer type is signed and many platforms use 32bit integers, filesize()
 *   may return unexpected results for files which are larger than 2GB. For
 *   files between 2GB and 4GB in size this can usually be overcome by using
 *   sprintf("%u", filesize($file)).
 *
 */
<<__Native>>
function filesize(string $filename): mixed;

/**
 * Gets the file owner.
 *
 * @param string $filename - Path to the file.
 *
 * @return mixed - Returns the user ID of the owner of the file, or FALSE on
 *   failure. The user ID is returned in numerical format, use posix_getpwuid()
 *   to resolve it to a username.
 *
 */
<<__Native>>
function fileowner(string $filename): mixed;

/**
 * Gets the file group. The group ID is returned in numerical format, use
 *   posix_getgrgid() to resolve it to a group name.
 *
 * @param string $filename - Path to the file.
 *
 * @return mixed - Returns the group ID of the file, or FALSE in case of an
 *   error. The group ID is returned in numerical format, use posix_getgrgid()
 *   to resolve it to a group name. Upon failure, FALSE is returned.
 *
 */
<<__Native>>
function filegroup(string $filename): mixed;

/**
 * @param string $filename - Path to the file.
 *
 * @return mixed - Returns the time the file was last accessed, or FALSE on
 *   failure. The time is returned as a Unix timestamp.
 *
 */
<<__Native>>
function fileatime(string $filename): mixed;

/**
 * This function returns the time when the data blocks of a file were being
 *   written to, that is, the time when the content of the file was changed.
 *
 * @param string $filename - Path to the file.
 *
 * @return mixed - Returns the time the file was last modified, or FALSE on
 *   failure. The time is returned as a Unix timestamp, which is suitable for
 *   the date() function.
 *
 */
<<__Native>>
function filemtime(string $filename): mixed;

/**
 * Gets the inode change time of a file.
 *
 * @param string $filename - Path to the file.
 *
 * @return mixed - Returns the time the file was last changed, or FALSE on
 *   failure. The time is returned as a Unix timestamp.
 *
 */
<<__Native>>
function filectime(string $filename): mixed;

/**
 * Returns the type of the given file.
 *
 * @param string $filename - Path to the file.
 *
 * @return mixed - Returns the type of the file. Possible values are fifo,
 *   char, dir, block, link, file, socket and unknown.  Returns FALSE if an
 *   error occurs. filetype() will also produce an E_NOTICE message if the stat
 *   call fails or if the file type is unknown.
 *
 */
<<__Native>>
function filetype(string $filename): mixed;

/**
 * Gets information about a link.  This function is used to verify if a link
 *   (pointed to by path) really exists (using the same method as the S_ISLNK
 *   macro defined in stat.h).
 *
 * @param string $filename - Path to the link.
 *
 * @return mixed - linkinfo() returns the st_dev field of the Unix C stat
 *   structure returned by the lstat system call. Returns 0 or FALSE in case of
 *   error.
 *
 */
<<__Native>>
function linkinfo(string $filename): mixed;

/**
 * Returns TRUE if the filename exists and is writable. The filename argument
 *   may be a directory name allowing you to check if a directory is writable.
 *   Keep in mind that PHP may be accessing the file as the user id that the web
 *   server runs as (often 'nobody'). Safe mode limitations are not taken into
 *   account.
 *
 * @param string $filename - The filename being checked.
 *
 * @return bool - Returns TRUE if the filename exists and is writable.
 *
 */
<<__Native>>
function is_writable(string $filename): bool;

<<__Native>>
function is_writeable(string $filename): bool;

/**
 * Tells whether a file exists and is readable.
 *
 * @param string $filename - Path to the file.
 *
 * @return bool - Returns TRUE if the file or directory specified by filename
 *   exists and is readable, FALSE otherwise.
 *
 */
<<__Native>>
function is_readable(string $filename): bool;

/**
 * Tells whether the filename is executable.
 *
 * @param string $filename - Path to the file.
 *
 * @return bool - Returns TRUE if the filename exists and is executable, or
 *   FALSE on error.
 *
 */
<<__Native>>
function is_executable(string $filename): bool;

/**
 * Tells whether the given file is a regular file.
 *
 * @param string $filename - Path to the file.
 *
 * @return bool - Returns TRUE if the filename exists and is a regular file,
 *   FALSE otherwise.
 *
 */
<<__Native>>
function is_file(string $filename): bool;

/**
 * Tells whether the given filename is a directory.
 *
 * @param string $filename - Path to the file. If filename is a relative
 *   filename, it will be checked relative to the current working directory. If
 *   filename is a symbolic or hard link then the link will be resolved and
 *   checked. If you have enabled safe mode, or open_basedir further
 *   restrictions may apply.
 *
 * @return bool - Returns TRUE if the filename exists and is a directory,
 *   FALSE otherwise.
 *
 */
<<__Native>>
function is_dir(string $filename): bool;

/**
 * Tells whether the given file is a symbolic link.
 *
 * @param string $filename - Path to the file.
 *
 * @return bool - Returns TRUE if the filename exists and is a symbolic link,
 *   FALSE otherwise.
 *
 */
<<__Native>>
function is_link(string $filename): bool;

/**
 * Returns TRUE if the file named by filename was uploaded via HTTP POST. This
 *   is useful to help ensure that a malicious user hasn't tried to trick the
 *   script into working on files upon which it should not be working--for
 *   instance, /etc/passwd.  This sort of check is especially important if there
 *   is any chance that anything done with uploaded files could reveal their
 *   contents to the user, or even to other users on the same system.  For
 *   proper working, the function is_uploaded_file() needs an argument like
 *   $_FILES['userfile']['tmp_name'], - the name of the uploaded file on the
 *   clients machine $_FILES['userfile']['name'] does not work.
 *
 * @param string $filename - The filename being checked.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function is_uploaded_file(string $filename): bool;

/**
 * Checks whether a file or directory exists.
 *
 * @param string $filename - Path to the file or directory.  On windows, use
 *   //computername/share/filename or \\computername\share\filename to check
 *   files on network shares.
 *
 * @return bool - Returns TRUE if the file or directory specified by filename
 *   exists; FALSE otherwise.  This function will return FALSE for symlinks
 *   pointing to non-existing files. Warning  This function returns FALSE for
 *   files inaccessible due to safe mode restrictions. However these files still
 *   can be included if they are located in safe_mode_include_dir.  The check is
 *   done using the real UID/GID instead of the effective one.
 *
 */
<<__Native>>
function file_exists(string $filename): bool;

/**
 * Gathers the statistics of the file named by filename. If filename is a
 *   symbolic link, statistics are from the file itself, not the symlink.
 *   lstat() is identical to stat() except it would instead be based off the
 *   symlinks status.
 *
 * @param string $filename - Path to the file.
 *
 * @return mixed - stat() and fstat() result format Numeric Associative (since
 *   PHP 4.0.6) Description 0 dev device number 1 ino inode number * 2 mode
 *   inode protection mode 3 nlink number of links 4 uid userid of owner * 5 gid
 *   groupid of owner * 6 rdev device type, if inode device 7 size size in bytes
 *   8 atime time of last access (Unix timestamp) 9 mtime time of last
 *   modification (Unix timestamp) 10 ctime time of last inode change (Unix
 *   timestamp) 11 blksize blocksize of filesystem IO ** 12 blocks number of
 *   512-byte blocks allocated ** * On Windows this will always be 0.  ** Only
 *   valid on systems supporting the st_blksize type - other systems (e.g.
 *   Windows) return -1.  In case of error, stat() returns FALSE.
 *
 */
<<__Native>>
function stat(string $filename): mixed;

/**
 * Gathers the statistics of the file or symbolic link named by filename.
 *
 * @param string $filename - Path to a file or a symbolic link.
 *
 * @return mixed - See the manual page for stat() for information on the
 *   structure of the array that lstat() returns. This function is identical to
 *   the stat() function except that if the filename parameter is a symbolic
 *   link, the status of the symbolic link is returned, not the status of the
 *   file pointed to by the symbolic link.
 *
 */
<<__Native>>
function lstat(string $filename): mixed;

/**
 * When you use stat(), lstat(), or any of the other functions listed in the
 *   affected functions list (below), PHP caches the information those functions
 *   return in order to provide faster performance. However, in certain cases,
 *   you may want to clear the cached information. For instance, if the same
 *   file is being checked multiple times within a single script, and that file
 *   is in danger of being removed or changed during that script's operation,
 *   you may elect to clear the status cache. In these cases, you can use the
 *   clearstatcache() function to clear the information that PHP caches about a
 *   file.  You should also note that PHP doesn't cache information about
 *   non-existent files. So, if you call file_exists() on a file that doesn't
 *   exist, it will return FALSE until you create the file. If you create the
 *   file, it will return TRUE even if you then delete the file. However
 *   unlink() clears the cache automatically.  This function caches information
 *   about specific filenames, so you only need to call clearstatcache() if you
 *   are performing multiple operations on the same filename and require the
 *   information about that particular file to not be cached.  Affected
 *   functions include stat(), lstat(), file_exists(), is_writable(),
 *   is_readable(), is_executable(), is_file(), is_dir(), is_link(),
 *   filectime(), fileatime(), filemtime(), fileinode(), filegroup(),
 *   fileowner(), filesize(), filetype(), and fileperms().
 *
 * @param bool $clear_realpath_cache - Whether to clear the realpath cache or
 *   not.
 * @param string $filename - Clear the realpath and the stat cache for a
 *   specific filename only; only used if clear_realpath_cache is TRUE.
 *
 */
<<__Native>>
function clearstatcache(bool $clear_realpath_cache = false,
                        ?string $filename = null): void;

/**
 * readlink() does the same as the readlink C function.
 *
 * @param string $path - The symbolic link path.
 *
 * @return mixed - Returns the contents of the symbolic link path or FALSE on
 *   error.
 *
 */
<<__Native>>
function readlink(string $path): mixed;

/**
 * realpath() expands all symbolic links and resolves references to '/./',
 *   '/../' and extra '/' characters in the input path and return the
 *   canonicalized absolute pathname.
 *
 * @param string $path - The path being checked.
 *
 * @return mixed - Returns the canonicalized absolute pathname on success. The
 *   resulting path will have no symbolic link, '/./' or '/../' components.
 *   realpath() returns FALSE on failure, e.g. if the file does not exist.  The
 *   running script must have executable permissions on all directories in the
 *   hierarchy, otherwise realpath() will return FALSE.
 *
 */
<<__Native>>
function realpath(string $path)[read_globals]: mixed;

/**
 * pathinfo() returns an associative array containing information about path.
 *
 * @param string $path - The path being checked.
 *
 * @param int $opt - You can specify which elements are returned with optional
 *   parameter options. It composes from PATHINFO_DIRNAME, PATHINFO_BASENAME,
 *   PATHINFO_EXTENSION and PATHINFO_FILENAME. It defaults to return all
 *   elements.
 *
 * @return mixed - The following associative array elements are returned:
 *   dirname, basename, extension (if any), and filename.  If options is used,
 *   this function will return a string if not all elements are requested.
 *
 */
<<__Native>>
function pathinfo(string $path, int $opt = 15): mixed;

/**
 * Given a string containing a directory, this function will return the number
 *   of bytes available on the corresponding filesystem or disk partition.
 *
 * @param string $directory - A directory of the filesystem or disk partition.
 *    Given a file name instead of a directory, the behaviour of the function is
 *   unspecified and may differ between operating systems and PHP versions.
 *
 * @return mixed - Returns the number of available bytes as a float or FALSE
 *   on failure.
 *
 */
<<__Native>>
function disk_free_space(string $directory): mixed;

<<__Native>>
function diskfreespace(string $directory): mixed;

/**
 * Given a string containing a directory, this function will return the total
 *   number of bytes on the corresponding filesystem or disk partition.
 *
 * @param string $directory - A directory of the filesystem or disk partition.
 *
 * @return mixed - Returns the total number of bytes as a float or FALSE on
 *   failure.
 *
 */
<<__Native>>
function disk_total_space(string $directory): mixed;

/**
 * Attempts to create the directory specified by pathname.
 *
 * @param string $pathname - The directory path.
 * @param int $mode - The mode is 0777 by default, which means the widest
 *   possible access. For more information on modes, read the details on the
 *   chmod() page.  mode is ignored on Windows.  Note that you probably want to
 *   specify the mode as an octal number, which means it should have a leading
 *   zero. The mode is also modified by the current umask, which you can change
 *   using umask().
 * @param bool $recursive - Allows the creation of nested directories
 *   specified in the pathname. Defaults to FALSE.
 * @param mixed $context - Context support was added with PHP 5.0.0. For a
 *   description of contexts, refer to Stream Functions.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function mkdir(string $pathname,
               int $mode = 0777,
               bool $recursive = false,
               mixed $context = null): bool;

/**
 * Attempts to remove the directory named by dirname. The directory must be
 *   empty, and the relevant permissions must permit this. A E_WARNING level
 *   error will be generated on failure.
 *
 * @param string $dirname - Path to the directory.
 * @param mixed $context - Context support was added with PHP 5.0.0. For a
 *   description of contexts, refer to Stream Functions.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function rmdir(string $dirname, mixed $context = null): bool;

/**
 * Given a string containing a path to a file, this function will return the
 *   name of the directory.
 *
 * @param string $path - A path.  On Windows, both slash (/) and backslash (\)
 *   are used as directory separator character. In other environments, it is the
 *   forward slash (/).
 *
 * @return string - Returns the name of the directory. If there are no slashes
 *   in path, a dot ('.') is returned, indicating the current directory.
 *   Otherwise, the returned string is path with any trailing /component
 *   removed.
 *
 */
<<__Native, __IsFoldable>>
function dirname(string $path)[]: string;

/**
 * Gets the current working directory.
 *
 * @return mixed - Returns the current working directory on success, or FALSE
 *   on failure.  On some Unix variants, getcwd() will return FALSE if any one
 *   of the parent directories does not have the readable or search mode set,
 *   even if the current directory does. See chmod() for more information on
 *   modes and permissions.
 *
 */
<<__Native>>
function getcwd(): mixed;

/**
 * Changes PHP's current directory to directory.
 *
 * @param string $directory - The new current directory
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function chdir(string $directory): bool;

/**
 * Changes the root directory of the current process to directory.  This
 *   function is only available if your system supports it and you're using the
 *   CLI, CGI or Embed SAPI. Also, this function requires root privileges.
 *
 * @param string $directory - The new directory
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function chroot(string $directory): bool;

/**
 * @param string $directory
 *
 * @return mixed
 *
 */
<<__Native>>
function dir(string $directory): mixed;

/**
 * Opens up a directory handle to be used in subsequent closedir(), readdir(),
 *   and rewinddir() calls.
 *
 * @param string $path - The directory path that is to be opened
 * @param mixed $context - For a description of the context parameter, refer
 *   to the streams section of the manual.
 *
 * @return mixed - Returns a directory handle resource on success, or FALSE on
 *   failure.  If path is not a valid directory or the directory can not be
 *   opened due to permission restrictions or filesystem errors, opendir()
 *   returns FALSE and generates a PHP error of level E_WARNING. You can
 *   suppress the error output of opendir() by prepending '@' to the front of
 *   the function name.
 *
 */
<<__Native>>
function opendir(string $path, mixed $context = null): mixed;

/**
 * Returns the filename of the next file from the directory. The filenames are
 *   returned in the order in which they are stored by the filesystem.
 *
 * @param resource $dir_handle - The directory handle resource previously
 *   opened with opendir(). If the directory handle is not specified, the last
 *   link opened by opendir() is assumed.
 *
 * @return mixed - Returns the filename on success or FALSE on failure.
 *   WarningThis function may return Boolean FALSE, but may also return a
 *   non-Boolean value which evaluates to FALSE, such as 0 or "". Please read
 *   the section on Booleans for more information. Use the === operator for
 *   testing the return value of this function.
 *
 */
<<__Native>>
function readdir(?resource $dir_handle = null): mixed;

/**
 * Resets the directory stream indicated by dir_handle to the beginning of the
 *   directory.
 *
 * @param resource $dir_handle
 *
 */
<<__Native>>
function rewinddir(?resource $dir_handle = null): void;

/**
 * Returns an array of files and directories from the directory.
 *
 * @param string $directory - The directory that will be scanned.
 * @param bool $descending - By default, the sorted order is alphabetical in
 *   ascending order. If the optional sorting_order is set to non-zero, then the
 *   sort order is alphabetical in descending order.
 * @param mixed $context - For a description of the context parameter, refer
 *   to the streams section of the manual.
 *
 * @return mixed - Returns an array of filenames on success, or FALSE on
 *   failure. If directory is not a directory, then boolean FALSE is returned,
 *   and an error of level E_WARNING is generated.
 *
 */
<<__Native>>
function scandir(string $directory,
                 bool $descending = false,
                 mixed $context = null): mixed;

/**
 * Closes the directory stream indicated by dir_handle. The stream must have
 *   previously been opened by opendir().
 *
 * @param resource $dir_handle - The directory handle resource previously
 *   opened with opendir(). If the directory handle is not specified, the last
 *   link opened by opendir() is assumed.
 *
 */
<<__Native>>
function closedir(?resource $dir_handle = null): void;

}

namespace HH {

/**
 * Return a stream resource attached to stdin in script environments,
 * or null in web requests
 */
<<__Native>>
function try_stdin()[]: ?resource;

/**
 * Return a stream resource attached to stdout in script environments,
 * or null in web requests
 */
<<__Native>>
function try_stdout()[]: ?resource;

/**
 * Return a stream resource attached to stderr in script environments,
 * or null in web requests
 */
<<__Native>>
function try_stderr()[]: ?resource;

/**
 * Return a stream resource attached to stdin in script environments,
 * or throw a RuntimeException in web requests.
 */
function stdin()[]: resource {
  $s = try_stdin();
  if ($s is null) {
    throw new \RuntimeException(
      "Request STDIO file descriptors are only available in CLI mode"
    );
  }
  return $s;
}

/**
 * Return a stream resource attached to stdout in script environments,
 * or throw a RuntimeException in web requests.
 */
function stdout()[]: resource {
  $s = try_stdout();
  if ($s is null) {
    throw new \RuntimeException(
      "Request STDIO file descriptors are only available in CLI mode"
    );
  }
  return $s;
}

/**
 * Return a stream resource attached to stderr in script environments,
 * or throw a RuntimeException in web requests.
 */
function stderr()[]: resource {
  $s = try_stderr();
  if ($s is null) {
    throw new \RuntimeException(
      "Request STDIO file descriptors are only available in CLI mode"
    );
  }
  return $s;
}

}
