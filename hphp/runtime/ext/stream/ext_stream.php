<?hh

/**
 * Creates and returns a stream context with any options supplied in options
 *   preset.
 *
 * @param array $options - Must be an associative array of associative arrays
 *   in the format $arr['wrapper']['option'] = $value.  Default to an empty
 *   array.
 * @param array $params - Must be an associative array in the format
 *   $arr['parameter'] = $value. Refer to context parameters for a listing of
 *   standard stream parameters.
 *
 * @return mixed - A stream context resource.
 *
 */
<<__Native>>
function stream_context_create(?array $options = null,
                               ?array $params = null): mixed;

/**
 * @param array $options - The options to set for the default context.
 *   options must be an associative array of associative arrays in the format
 *   $arr['wrapper']['option'] = $value.  Refer to context options and
 *   parameters for a listing of stream options.
 *
 * @return mixed - Returns the default stream context.
 *
 */
function stream_context_set_default(?array $options = null): mixed {
  return stream_context_get_default($options);
}

/**
 * @param array $options - The options to set for the default context.
 *   options must be an associative array of associative arrays in the format
 *   $arr['wrapper']['option'] = $value.  Refer to context options and
 *   parameters for a listing of stream options.
 *
 * @return mixed - Returns the default stream context.
 *
 */
<<__Native>>
function stream_context_get_default(?array $options = null): mixed;

/**
 * @param resource $stream_or_context - The stream or context to get options
 *   from
 *
 * @return mixed - Returns an associative array with the options.
 *
 */
<<__Native>>
function stream_context_get_options(resource $stream_or_context): mixed;

/**
 * @param mixed $stream_or_context - The stream or context resource to apply
 *   the options too.
 * @param mixed $wrapper_or_options - The options to set for the default
 *   context.  options must be an associative array of associative arrays in the
 *   format $arr['wrapper']['option'] = $value.  Refer to context options and
 *   parameters for a listing of stream options.
 * @param mixed $option
 * @param mixed $value
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function stream_context_set_option(mixed $stream_or_context = null,
                                   mixed $wrapper_or_options = null,
                                   mixed $option = null,
                                   mixed $value = null): bool;

<<__Native>>
function stream_context_get_params(resource $stream_or_context): mixed;

<<__Native>>
function stream_context_set_params(resource $stream_or_context,
                                   array $params): bool;

/**
 * Makes a copy of up to maxlength bytes of data from the current position (or
 *   from the offset position, if specified) in source to dest. If maxlength is
 *   not specified, all remaining content in source will be copied.
 *
 * @param resource $source - The source stream
 * @param resource $dest - The destination stream
 * @param int $maxlength - Maximum bytes to copy
 * @param int $offset - The offset where to start to copy data
 *
 * @return mixed - Returns the total count of bytes copied.
 *
 */
<<__Native>>
function stream_copy_to_stream(resource $source,
                               resource $dest,
                               int $maxlength = -1,
                               int $offset = 0): mixed;

/**
 * Identical to file_get_contents(), except that stream_get_contents()
 *   operates on an already open stream resource and returns the remaining
 *   contents in a string, up to maxlength bytes and starting at the specified
 *   offset.
 *
 * @param resource $handle - A stream resource (e.g. returned from fopen())
 * @param int $maxlen - The maximum bytes to read. Defaults to -1 (read all
 *   the remaining buffer).
 * @param int $offset - Seek to the specified offset before reading. Defaults
 *   to -1 (don't seek).
 *
 * @return mixed - Returns a string or FALSE on failure.
 *
 */
<<__Native, __ParamCoerceModeFalse>>
function stream_get_contents(resource $handle,
                             int $maxlen = -1,
                             int $offset = -1): mixed;

/**
 * Gets a line from the given handle.  Reading ends when length bytes have
 *   been read, when the string specified by ending is found (which is not
 *   included in the return value), or on EOF (whichever comes first).  This
 *   function is nearly identical to fgets() except in that it allows end of
 *   line delimiters other than the standard \n, \r, and \r\n, and does not
 *   return the delimiter itself.
 *
 * @param resource $handle - A valid file handle.
 * @param int $length - The number of bytes to read from the handle.
 * @param string $ending - An optional string delimiter.
 *
 * @return mixed - Returns a string of up to length bytes read from the file
 *   pointed to by handle.  If an error occurs, returns FALSE.
 *
 */
<<__Native>>
function stream_get_line(resource $handle,
                         int $length = 0,
                         ?string $ending = null): mixed;

/**
 * Returns information about an existing stream.
 *
 * @param resource $stream - The stream can be any stream created by fopen(),
 *   fsockopen() and pfsockopen().
 *
 * @return mixed - The result array contains the following items:  timed_out
 *   (bool) - TRUE if the stream timed out while waiting for data on the last
 *   call to fread() or fgets().  blocked (bool) - TRUE if the stream is in
 *   blocking IO mode. See stream_set_blocking().  eof (bool) - TRUE if the
 *   stream has reached end-of-file. Note that for socket streams this member
 *   can be TRUE even when unread_bytes is non-zero. To determine if there is
 *   more data to be read, use feof() instead of reading this item.
 *   unread_bytes (int) - the number of bytes currently contained in the PHP's
 *   own internal buffer. You shouldn't use this value in a script.  stream_type
 *   (string) - a label describing the underlying implementation of the stream.
 *   wrapper_type (string) - a label describing the protocol wrapper
 *   implementation layered over the stream. See List of Supported
 *   Protocols/Wrappers for more information about wrappers.  wrapper_data
 *   (mixed) - wrapper specific data attached to this stream. See List of
 *   Supported Protocols/Wrappers for more information about wrappers and their
 *   wrapper data.  filters (array) - and array containing the names of any
 *   filters that have been stacked onto this stream. Documentation on filters
 *   can be found in the Filters appendix.  mode (string) - the type of access
 *   required for this stream (see Table 1 of the fopen() reference)  seekable
 *   (bool) - whether the current stream can be seeked.  uri (string) - the
 *   URI/filename associated with this stream.
 *
 */
<<__Native>>
function stream_get_meta_data(resource $stream): mixed;

/**
 * @return array - Returns an indexed array of socket transports names.
 *
 */
<<__Native>>
function stream_get_transports(): array;

/**
 * Retrieve list of registered streams available on the running system.
 *
 * @return array - Returns an indexed array containing the name of all stream
 *   wrappers available on the running system.
 *
 */
<<__Native>>
function stream_get_wrappers(): array;

/**
 * Checks if a stream is a local stream
 *
 * @param mixed $stream_or_url
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function stream_is_local(mixed $stream_or_url): bool;

/**
 * This function is an alias of: stream_wrapper_register().
 *
 */
<<__Native>>
function stream_register_wrapper(string $protocol,
                                 string $classname,
                                 int $flags = 0): bool;

/**
 * Allows you to implement your own protocol handlers and streams for use with
 *   all the other filesystem functions (such as fopen(), fread() etc.).
 *
 * @param string $protocol - The wrapper name to be registered.
 * @param string $classname - The classname which implements the protocol.
 * @param int $flags - Should be set to STREAM_IS_URL if protocol is a URL
 *   protocol. Default is 0, local stream.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *   stream_wrapper_register() will return FALSE if the protocol already has a
 *   handler.
 *
 */
<<__Native>>
function stream_wrapper_register(string $protocol,
                                 string $classname,
                                 int $flags = 0): bool;

/**
 * Restores a built-in wrapper previously unregistered with
 *   stream_wrapper_unregister().
 *
 * @param string $protocol
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function stream_wrapper_restore(string $protocol): bool;

/**
 * Allows you to disable an already defined stream wrapper. Once the wrapper
 *   has been disabled you may override it with a user-defined wrapper using
 *   stream_wrapper_register() or reenable it later on with
 *   stream_wrapper_restore().
 *
 * @param string $protocol
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function stream_wrapper_unregister(string $protocol): bool;

/**
 * Resolve filename against the include path according to the same rules as
 *   fopen()/include() does.
 *
 * @param string $filename - The filename to resolve.
 * @param resource $context - A valid context resource created with
 *   stream_context_create().
 *
 * @return mixed - On success, the resolved absolute filename is returned. On
 *   failure, FALSE is returned.
 *
 */
<<__Native>>
function stream_resolve_include_path(string $filename,
                                     ?resource $context = null): mixed;

/**
 * The stream_select() function accepts arrays of streams and waits for them
 *   to change status. Its operation is equivalent to that of the
 *   socket_select() function except in that it acts on streams.
 *
 * @param mixed $read - The streams listed in the read array will be watched
 *   to see if characters become available for reading (more precisely, to see
 *   if a read will not block - in particular, a stream resource is also ready
 *   on end-of-file, in which case an fread() will return a zero length string).
 * @param mixed $write - The streams listed in the write array will be watched
 *   to see if a write will not block.
 * @param mixed $except - The streams listed in the except array will be
 *   watched for high priority exceptional ("out-of-band") data arriving.  When
 *   stream_select() returns, the arrays read, write and except are modified to
 *   indicate which stream resource(s) actually changed status. You do not need
 *   to pass every array to stream_select(). You can leave it out and use an
 *   empty array or NULL instead. Also do not forget that those arrays are
 *   passed by reference and will be modified after stream_select() returns.
 * @param mixed $vtv_sec - The tv_sec and tv_usec together form the timeout
 *   parameter, tv_sec specifies the number of seconds while tv_usec the number
 *   of microseconds. The timeout is an upper bound on the amount of time that
 *   stream_select() will wait before it returns. If tv_sec and tv_usec are both
 *   set to 0, stream_select() will not wait for data - instead it will return
 *   immediately, indicating the current status of the streams.  If tv_sec is
 *   NULL stream_select() can block indefinitely, returning only when an event
 *   on one of the watched streams occurs (or if a signal interrupts the system
 *   call). Warning  Using a timeout value of 0 allows you to instantaneously
 *   poll the status of the streams, however, it is NOT a good idea to use a 0
 *   timeout value in a loop as it will cause your script to consume too much
 *   CPU time.  It is much better to specify a timeout value of a few seconds,
 *   although if you need to be checking and running other code concurrently,
 *   using a timeout value of at least 200000 microseconds will help reduce the
 *   CPU usage of your script.  Remember that the timeout value is the maximum
 *   time that will elapse; stream_select() will return as soon as the requested
 *   streams are ready for use.
 * @param int $tv_usec - See tv_sec description.
 *
 * @return mixed - On success stream_select() returns the number of stream
 *   resources contained in the modified arrays, which may be zero if the
 *   timeout expires before anything interesting happens. On error FALSE is
 *   returned and a warning raised (this can happen if the system call is
 *   interrupted by an incoming signal).
 *
 */
<<__Native>>
function stream_select(mixed &$read,
                       mixed &$write,
                       mixed &$except,
                       mixed $vtv_sec,
                       int $tv_usec = 0): mixed;

/**
 * Awaitable version of stream_select()
 *
 * @param resource $fp - Stream resource, must be backed by a file descriptor
 *                       such as a normal file, socket, tempfile, or stdio.
 *                       Does not work with memory streams or user streams.
 * @param int $events - Mix of STREAM_AWAIT_READ and/or STREAM_EVENT_WRITE
 * @param float $timeout - Timeout in seconds
 *
 * @return int - Result code
 *               STREAM_AWAIT_CLOSED: Stream is closed
 *               STREAM_AWAIT_READY: Activity on the provided stream
 *               STREAM_AWAIT_TIMEOUT: No activity (timeout occured)
 *               STREAM_AWAIT_ERROR: Error
 */
<<__Native>>
async function stream_await(resource $fp,
                            int $events,
                            float $timeout = 0.0): Awaitable<int>;

/**
 * Sets blocking or non-blocking mode on a stream.  This function works for
 *   any stream that supports non-blocking mode (currently, regular files and
 *   socket streams).
 *
 * @param resource $stream - The stream.
 * @param int $mode - If mode is 0, the given stream will be switched to
 *   non-blocking mode, and if 1, it will be switched to blocking mode. This
 *   affects calls like fgets() and fread() that read from the stream. In
 *   non-blocking mode an fgets() call will always return right away while in
 *   blocking mode it will wait for data to become available on the stream.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function stream_set_blocking(resource $stream, int $mode): bool;

/**
 * Sets the timeout value on stream, expressed in the sum of seconds and
 *   microseconds.  When the stream times out, the 'timed_out' key of the array
 *   returned by stream_get_meta_data() is set to TRUE, although no
 *   error/warning is generated.
 *
 * @param resource $stream - The target stream.
 * @param int $seconds - The seconds part of the timeout to be set.
 * @param int $microseconds - The microseconds part of the timeout to be set.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function stream_set_timeout(resource $stream,
                            int $seconds,
                            int $microseconds = 0): bool;

/**
 * Sets the buffering for write operations on the given stream to buffer
 *   bytes. Output using fwrite() is normally buffered at 8K. This means that if
 *   there are two processes wanting to write to the same output stream (a
 *   file), each is paused after 8K of data to allow the other to write.
 *
 * @param resource $stream - The file pointer.
 * @param int $buffer - The number of bytes to buffer. If buffer is 0 then
 *   write operations are unbuffered. This ensures that all writes with fwrite()
 *   are completed before other processes are allowed to write to that output
 *   stream.
 *
 * @return int - Returns 0 on success, or EOF if the request cannot be
 *   honored.
 *
 */
<<__Native>>
function stream_set_write_buffer(resource $stream, int $buffer): int;

<<__Native>>
function set_file_buffer(resource $stream, int $buffer): int;

/**
 * Accept a connection on a socket previously created by
 *   stream_socket_server().
 *
 * @param resource $server_socket - The server socket to accept a connection
 *   from.
 * @param float $timeout - Override the default socket accept timeout. Time
 *   should be given in seconds.
 * @param mixed $peername - Will be set to the name (address) of the client
 *   which connected, if included and available from the selected transport.
 *   Can also be determined later using stream_socket_get_name().
 *
 * @return mixed - Returns a stream to the accepted socket connection or FALSE
 *   on failure.
 *
 */
<<__Native>>
function stream_socket_accept(resource $server_socket,
                              float $timeout = -1.0,
                              mixed &$peername = null): mixed;

/**
 * Creates a stream or datagram socket on the specified local_socket.  This
 *   function only creates a socket, to begin accepting connections use
 *   stream_socket_accept().
 *
 * @param string $local_socket - The type of socket created is determined by
 *   the transport specified using standard URL formatting: transport://target.
 *   For Internet Domain sockets (AF_INET) such as TCP and UDP, the target
 *   portion of the remote_socket parameter should consist of a hostname or IP
 *   address followed by a colon and a port number. For Unix domain sockets, the
 *   target portion should point to the socket file on the filesystem.
 *   Depending on the environment, Unix domain sockets may not be available. A
 *   list of available transports can be retrieved using
 *   stream_get_transports(). See List of Supported Socket Transports for a list
 *   of builtin transports.
 * @param mixed $errnum - If the optional errno and errstr arguments are
 *   present they will be set to indicate the actual system level error that
 *   occurred in the system-level socket(), bind(), and listen() calls. If the
 *   value returned in errno is 0 and the function returned FALSE, it is an
 *   indication that the error occurred before the bind() call. This is most
 *   likely due to a problem initializing the socket. Note that the errno and
 *   errstr arguments will always be passed by reference.
 * @param mixed $errstr - See errno description.
 * @param int $flags - A bitmask field which may be set to any combination of
 *   socket creation flags.  For UDP sockets, you must use STREAM_SERVER_BIND as
 *   the flags parameter.
 * @param resource $context
 *
 * @return mixed - Returns the created stream, or FALSE on error.
 *
 */
<<__Native>>
function stream_socket_server(string $local_socket,
                              mixed &$errnum = null,
                              mixed &$errstr = null,
                              int $flags = STREAM_SERVER_BIND |
                                STREAM_SERVER_LISTEN,
                              ?resource $context = null): mixed;

/**
 * Initiates a stream or datagram connection to the destination specified by
 *   remote_socket. The type of socket created is determined by the transport
 *   specified using standard URL formatting: transport://target. For Internet
 *   Domain sockets (AF_INET) such as TCP and UDP, the target portion of the
 *   remote_socket parameter should consist of a hostname or IP address followed
 *   by a colon and a port number. For Unix domain sockets, the target portion
 *   should point to the socket file on the filesystem.  The stream will by
 *   default be opened in blocking mode. You can switch it to non-blocking mode
 *   by using stream_set_blocking().
 *
 * @param string $remote_socket - Address to the socket to connect to.
 * @param mixed $errnum - Will be set to the system level error number if
 *   connection fails.
 * @param mixed $errstr - Will be set to the system level error message if the
 *   connection fails.
 * @param float $timeout - Number of seconds until the connect() system call
 *   should timeout. This parameter only applies when not making asynchronous
 *   connection attempts.  To set a timeout for reading/writing data over the
 *   socket, use the stream_set_timeout(), as the timeout only applies while
 *   making connecting the socket.
 * @param int $flags - Bitmask field which may be set to any combination of
 *   connection flags. Currently the select of connection flags is limited to
 *   STREAM_CLIENT_CONNECT (default), STREAM_CLIENT_ASYNC_CONNECT and
 *   STREAM_CLIENT_PERSISTENT.
 * @param resource $context - A valid context resource created with
 *   stream_context_create().
 *
 * @return mixed - On success a stream resource is returned which may be used
 *   together with the other file functions (such as fgets(), fgetss(),
 *   fwrite(), fclose(), and feof()), FALSE on failure.
 *
 */
<<__Native>>
function stream_socket_client(string $remote_socket,
                              mixed &$errnum = null,
                              mixed &$errstr = null,
                              float $timeout = -1.0,
                              int $flags = 0,
                              ?resource $context = null): mixed;

/**
 * Turns encryption on/off on an already connected socket. Once the crypto
 * settings are established, cryptography can be turned on and off dynamically
 * by passing TRUE or FALSE in the enable parameter.
 *
 * @param resource $stream - The stream reszource.
 * @param bool $enable - Enable/disable cryptography on the stream.
 * @param int crypto_type - Setup encryption on the stream. Valid methods are:
 *   - STREAM_CRYPTO_SSLv2_CLIENT
 *   - STREAM_CRYPTO_SSLv3_CLIENT
 *   - STREAM_CRYPTO_SSLv23_CLIENT
 *   - STREAM_CRYPTO_TLS_CLIENT
 *  The following methods are valid, but not currently implemented in HHVM:
 *   - STREAM_CRYPTO_SSLv2_SERVER
 *   - STREAM_CRYPTO_SSLv3_SERVER
 *   - STREAM_CRYPTO_SSLv23_SERVER
 *   - STREAM_CRYPTO_TLS_SERVER
 *
 *   When enabling crypto in HHVM, this parameter is requried as the
 *   session_stream parameter is not supported.
 *
 *   Under PHP, if omitted, the crypto_type context option on the stream's SSL
 *   context will be used instead.
 * @param resource $session_stream Seed the stream with settings from
 *   session_stream. CURRENTLY UNSUPPORTED IN HHVM.
 *
 * @returns mixed - Returns TRUE on success, FALSE if negoation has failed, or
 *   0 if there isn't enough data and you should try again (only for
 *   non-blocking sockets).
 */
<<__Native>>
function stream_socket_enable_crypto(
  resource $socket,
  bool $enable,
  int $crypto_type = 0,
  ?resource $session_stream = null,
): bool;

/**
 * Returns the local or remote name of a given socket connection.
 *
 * @param resource $handle - The socket to get the name of.
 * @param bool $want_peer - If set to TRUE the remote socket name will be
 *   returned, if set to FALSE the local socket name will be returned.
 *
 * @return mixed - The name of the socket.
 *
 */
<<__Native>>
function stream_socket_get_name(resource $handle, bool $want_peer): mixed;

/**
 * stream_socket_pair() creates a pair of connected, indistinguishable socket
 *   streams. This function is commonly used in IPC (Inter-Process
 *   Communication). Please consult the Streams constant list for further
 *   details on each constant.
 *
 * @param int $domain - The protocol family to be used: STREAM_PF_INET,
 *   STREAM_PF_INET6 or STREAM_PF_UNIX
 * @param int $type - The type of communication to be used: STREAM_SOCK_DGRAM,
 *   STREAM_SOCK_RAW, STREAM_SOCK_RDM, STREAM_SOCK_SEQPACKET or
 *   STREAM_SOCK_STREAM
 * @param int $protocol - The protocol to be used: STREAM_IPPROTO_ICMP,
 *   STREAM_IPPROTO_IP, STREAM_IPPROTO_RAW, STREAM_IPPROTO_TCP or
 *   STREAM_IPPROTO_UDP
 *
 * @return mixed - Returns an array with the two socket resources on success,
 *   or FALSE on failure.
 *
 */
<<__Native>>
function stream_socket_pair(int $domain, int $type, int $protocol): mixed;

/**
 * stream_socket_recvfrom() accepts data from a remote socket up to length
 *   bytes.
 *
 * @param resource $socket - The remote socket.
 * @param int $length - The number of bytes to receive from the socket.
 * @param int $flags - The value of flags can be any combination of the
 *   following: Possible values for flags STREAM_OOB Process OOB (out-of-band)
 *   data. STREAM_PEEK Retrieve data from the socket, but do not consume the
 *   buffer. Subsequent calls to fread() or stream_socket_recvfrom() will see
 *   the same data.
 * @param mixed $address - If address is provided it will be populated with
 *   the address of the remote socket.
 *
 * @return mixed - Returns the read data, as a string
 *
 */
<<__Native>>
function stream_socket_recvfrom(resource $socket,
                                int $length,
                                int $flags = 0,
                                mixed &$address = null): mixed;

/**
 * Sends the specified data through the socket.
 *
 * @param resource $socket - The socket to send data to.
 * @param string $data - The data to be sent.
 * @param int $flags - The value of flags can be any combination of the
 *   following: possible values for flags STREAM_OOB Process OOB (out-of-band)
 *   data.
 * @param string $address - The address specified when the socket stream was
 *   created will be used unless an alternate address is specified in address.
 *   If specified, it must be in dotted quad (or [ipv6]) format.
 *
 * @return mixed - Returns a result code, as an integer.
 *
 */
<<__Native>>
function stream_socket_sendto(resource $socket,
                              string $data,
                              int $flags = 0,
                              ?string $address = null): mixed;

/**
 * Shutdowns (partially or not) a full-duplex connection.
 *
 * @param resource $stream - An open stream (opened with
 *   stream_socket_client(), for example)
 * @param int $how - One of the following constants: STREAM_SHUT_RD (disable
 *   further receptions), STREAM_SHUT_WR (disable further transmissions) or
 *   STREAM_SHUT_RDWR (disable further receptions and transmissions).
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function stream_socket_shutdown(resource $stream, int $how): bool;
