<?hh

/**
 * Creates and returns a socket resource, also referred to as an endpoint of
 *   communication. A typical network connection is made up of 2 sockets, one
 *   performing the role of the client, and another performing the role of the
 *   server.
 *
 * @param int $domain - The domain parameter specifies the protocol family to
 *   be used by the socket. Available address/protocol families Domain
 *   Description AF_INET IPv4 Internet based protocols. TCP and UDP are common
 *   protocols of this protocol family. AF_INET6 IPv6 Internet based protocols.
 *   TCP and UDP are common protocols of this protocol family. AF_UNIX Local
 *   communication protocol family. High efficiency and low overhead make it a
 *   great form of IPC (Interprocess Communication).
 * @param int $type - The type parameter selects the type of communication to
 *   be used by the socket. Available socket types Type Description SOCK_STREAM
 *   Provides sequenced, reliable, full-duplex, connection-based byte streams.
 *   An out-of-band data transmission mechanism may be supported. The TCP
 *   protocol is based on this socket type. SOCK_DGRAM Supports datagrams
 *   (connectionless, unreliable messages of a fixed maximum length). The UDP
 *   protocol is based on this socket type. SOCK_SEQPACKET Provides a sequenced,
 *   reliable, two-way connection-based data transmission path for datagrams of
 *   fixed maximum length; a consumer is required to read an entire packet with
 *   each read call. SOCK_RAW Provides raw network protocol access. This special
 *   type of socket can be used to manually construct any type of protocol. A
 *   common use for this socket type is to perform ICMP requests (like ping).
 *   SOCK_RDM Provides a reliable datagram layer that does not guarantee
 *   ordering. This is most likely not implemented on your operating system.
 * @param int $protocol - The protocol parameter sets the specific protocol
 *   within the specified domain to be used when communicating on the returned
 *   socket. The proper value can be retrieved by name by using
 *   getprotobyname(). If the desired protocol is TCP, or UDP the corresponding
 *   constants SOL_TCP, and SOL_UDP can also be used. Common protocols Name
 *   Description icmp The Internet Control Message Protocol is used primarily by
 *   gateways and hosts to report errors in datagram communication. The "ping"
 *   command (present in most modern operating systems) is an example
 *   application of ICMP. udp The User Datagram Protocol is a connectionless,
 *   unreliable, protocol with fixed record lengths. Due to these aspects, UDP
 *   requires a minimum amount of protocol overhead. tcp The Transmission
 *   Control Protocol is a reliable, connection based, stream oriented, full
 *   duplex protocol. TCP guarantees that all data packets will be received in
 *   the order in which they were sent. If any packet is somehow lost during
 *   communication, TCP will automatically retransmit the packet until the
 *   destination host acknowledges that packet. For reliability and performance
 *   reasons, the TCP implementation itself decides the appropriate octet
 *   boundaries of the underlying datagram communication layer. Therefore, TCP
 *   applications must allow for the possibility of partial record transmission.
 *
 * @return mixed - socket_create() returns a socket resource on success, or
 *   FALSE on error. The actual error code can be retrieved by calling
 *   socket_last_error(). This error code may be passed to socket_strerror() to
 *   get a textual explanation of the error.
 *
 */
<<__Native>>
function socket_create(int $domain, int $type, int $protocol)[leak_safe]: mixed;

/**
 * socket_create_listen() creates a new socket resource of type AF_INET
 *   listening on all local interfaces on the given port waiting for new
 *   connections.  This function is meant to ease the task of creating a new
 *   socket which only listens to accept new connections.
 *
 * @param int $port - The port on which to listen on all interfaces.
 * @param int $backlog - The backlog parameter defines the maximum length the
 *   queue of pending connections may grow to. SOMAXCONN may be passed as
 *   backlog parameter, see socket_listen() for more information.
 *
 * @return mixed - socket_create_listen() returns a new socket resource on
 *   success or FALSE on error. The error code can be retrieved with
 *   socket_last_error(). This code may be passed to socket_strerror() to get a
 *   textual explanation of the error.
 *
 */
<<__Native>>
function socket_create_listen(int $port, int $backlog = 128): mixed;

/**
 * socket_create_pair() creates two connected and indistinguishable sockets,
 *   and stores them in fd. This function is commonly used in IPC (InterProcess
 *   Communication).
 *
 * @param int $domain - The domain parameter specifies the protocol family to
 *   be used by the socket. See socket_create() for the full list.
 * @param int $type - The type parameter selects the type of communication to
 *   be used by the socket. See socket_create() for the full list.
 * @param int $protocol - The protocol parameter sets the specific protocol
 *   within the specified domain to be used when communicating on the returned
 *   socket. The proper value can be retrieved by name by using
 *   getprotobyname(). If the desired protocol is TCP, or UDP the corresponding
 *   constants SOL_TCP, and SOL_UDP can also be used.  See socket_create() for
 *   the full list of supported protocols.
 * @param mixed $fd - Reference to an array in which the two socket resources
 *   will be inserted.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function socket_create_pair(int $domain,
                            int $type,
                            int $protocol,
                            <<__OutOnly>>
                            inout mixed $fd): bool;

/**
 * The socket_get_option() function retrieves the value for the option
 *   specified by the optname parameter for the specified socket.
 *
 * @return mixed - Returns the value of the given option, or FALSE on errors.
 *
 */
<<__Native>>
function socket_get_option(resource $socket, int $level, int $optname)[leak_safe]: mixed;

/**
 * Queries the remote side of the given socket which may either result in
 *   host/port or in a Unix filesystem path, dependent on its type.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create() or socket_accept().
 * @param mixed $address - If the given socket is of type AF_INET or AF_INET6,
 *   socket_getpeername() will return the peers (remote) IP address in
 *   appropriate notation (e.g. 127.0.0.1 or fe80::1) in the address parameter
 *   and, if the optional port parameter is present, also the associated port.
 *   If the given socket is of type AF_UNIX, socket_getpeername() will return
 *   the Unix filesystem path (e.g. /var/run/daemon.sock) in the address
 *   parameter.
 * @param mixed $port - If given, this will hold the port associated to
 *   address.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *   socket_getpeername() may also return FALSE if the socket type is not any of
 *   AF_INET, AF_INET6, or AF_UNIX, in which case the last socket error code is
 *   not updated.
 *
 */
<<__Native>>
function socket_getpeername(resource $socket,
                            <<__OutOnly>>
                            inout mixed $address,
                            <<__OutOnly>>
                            inout mixed $port)[leak_safe]: bool;

/**
 * @param resource $socket - A valid socket resource created with
 *   socket_create() or socket_accept().
 *
 * @param mixed $address - If the given socket is of type AF_INET or AF_INET6,
 *   socket_getsockname() will return the local IP address in appropriate
 *   notation (e.g. 127.0.0.1 or fe80::1) in the address parameter and, if the
 *   optional port parameter is present, also the associated port.  If the given
 *   socket is of type AF_UNIX, socket_getsockname() will return the Unix
 *   filesystem path (e.g. /var/run/daemon.sock) in the address parameter.
 * @param mixed $port - If provided, this will hold the associated port.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *   socket_getsockname() may also return FALSE if the socket type is not any of
 *   AF_INET, AF_INET6, or AF_UNIX, in which case the last socket error code is
 *   not updated.
 *
 */
<<__Native>>
function socket_getsockname(resource $socket,
                            <<__OutOnly>>
                            inout mixed $address,
                            <<__OutOnly>>
                            inout mixed $port)[]: bool;

/**
 * The socket_set_block() function removes the O_NONBLOCK flag on the socket
 *   specified by the socket parameter.  When an operation (e.g. receive, send,
 *   connect, accept, ...) is performed on a blocking socket, the script will
 *   pause its execution until it receives a signal or it can perform the
 *   operation.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create() or socket_accept().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function socket_set_block(resource $socket)[leak_safe]: bool;

/**
 * The socket_set_nonblock() function sets the O_NONBLOCK flag on the socket
 *   specified by the socket parameter.  When an operation (e.g. receive, send,
 *   connect, accept, ...) is performed on a non-blocking socket, the script not
 *   pause its execution until it receives a signal or it can perform the
 *   operation. Rather, if the operation would result in a block, the called
 *   function will fail.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create() or socket_accept().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function socket_set_nonblock(resource $socket)[leak_safe]: bool;

/**
 * The socket_set_option() function sets the option specified by the optname
 *   parameter, at the specified protocol level, to the value pointed to by the
 *   optval parameter for the socket.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create() or socket_accept().
 * @param int $level - The level parameter specifies the protocol level at
 *   which the option resides. For example, to retrieve options at the socket
 *   level, a level parameter of SOL_SOCKET would be used. Other levels, such as
 *   TCP, can be used by specifying the protocol number of that level. Protocol
 *   numbers can be found by using the getprotobyname() function.
 * @param int $optname - The available socket options are the same as those
 *   for the socket_get_option() function.
 * @param mixed $optval - The option value.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function socket_set_option(resource $socket,
                           int $level,
                           int $optname,
                           mixed $optval)[leak_safe]: bool;

/**
 * Initiate a connection to address using the socket resource socket, which
 *   must be a valid socket resource created with socket_create().
 *
 * @param resource $socket
 * @param string $address - The address parameter is either an IPv4 address in
 *   dotted-quad notation (e.g. 127.0.0.1) if socket is AF_INET, a valid IPv6
 *   address (e.g. ::1) if IPv6 support is enabled and socket is AF_INET6 or the
 *   pathname of a Unix domain socket, if the socket family is AF_UNIX.
 * @param int $port - The port parameter is only used and is mandatory when
 *   connecting to an AF_INET or an AF_INET6 socket, and designates the port on
 *   the remote host to which a connection should be made.
 *
 * @return bool - Returns TRUE on success or FALSE on failure. The error code
 *   can be retrieved with socket_last_error(). This code may be passed to
 *   socket_strerror() to get a textual explanation of the error.  If the socket
 *   is non-blocking then this function returns FALSE with an error Operation
 *   now in progress.
 *
 */
<<__Native>>
function socket_connect(resource $socket, string $address, int $port = 0)[leak_safe]: bool;

/**
 * Binds the name given in address to the socket described by socket. This has
 *   to be done before a connection is be established using socket_connect() or
 *   socket_listen().
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create().
 * @param string $address - If the socket is of the AF_INET family, the
 *   address is an IP in dotted-quad notation (e.g. 127.0.0.1).  If the socket
 *   is of the AF_UNIX family, the address is the path of a Unix-domain socket
 *   (e.g. /tmp/my.sock).
 * @param int $port - The port parameter is only used when connecting to an
 *   AF_INET socket, and designates the port on the remote host to which a
 *   connection should be made.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.  The error code
 *   can be retrieved with socket_last_error(). This code may be passed to
 *   socket_strerror() to get a textual explanation of the error.
 *
 */
<<__Native>>
function socket_bind(resource $socket, string $address, int $port = 0): bool;

/**
 * After the socket socket has been created using socket_create() and bound to
 *   a name with socket_bind(), it may be told to listen for incoming
 *   connections on socket.  socket_listen() is applicable only to sockets of
 *   type SOCK_STREAM or SOCK_SEQPACKET.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create().
 * @param int $backlog - A maximum of backlog incoming connections will be
 *   queued for processing. If a connection request arrives with the queue full
 *   the client may receive an error with an indication of ECONNREFUSED, or, if
 *   the underlying protocol supports retransmission, the request may be ignored
 *   so that retries may succeed.  The maximum number passed to the backlog
 *   parameter highly depends on the underlying platform. On Linux, it is
 *   silently truncated to SOMAXCONN. On win32, if passed SOMAXCONN, the
 *   underlying service provider responsible for the socket will set the backlog
 *   to a maximum reasonable value. There is no standard provision to find out
 *   the actual backlog value on this platform.
 *
 * @return bool - Returns TRUE on success or FALSE on failure. The error code
 *   can be retrieved with socket_last_error(). This code may be passed to
 *   socket_strerror() to get a textual explanation of the error.
 *
 */
<<__Native>>
function socket_listen(resource $socket, int $backlog = 0): bool;

/**
 * socket_select() accepts arrays of sockets and waits for them to change
 *   status. Those coming with BSD sockets background will recognize that those
 *   socket resource arrays are in fact the so-called file descriptor sets.
 *   Three independent arrays of socket resources are watched. Warning  On exit,
 *   the arrays are modified to indicate which socket resource actually changed
 *   status.  You do not need to pass every array to socket_select(). You can
 *   leave it out and use an empty array or NULL instead. Also do not forget
 *   that those arrays are passed by reference and will be modified after
 *   socket_select() returns.  Due a limitation in the current Zend Engine it is
 *   not possible to pass a constant modifier like NULL directly as a parameter
 *   to a function which expects this parameter to be passed by reference.
 *   Instead use a temporary variable or an expression with the leftmost member
 *   being a temporary variable: Example #1 Using NULL with socket_select()
 *
 * @param mixed $read - The sockets listed in the read array will be watched
 *   to see if characters become available for reading (more precisely, to see
 *   if a read will not block - in particular, a socket resource is also ready
 *   on end-of-file, in which case a socket_read() will return a zero length
 *   string).
 * @param mixed $write - The sockets listed in the write array will be watched
 *   to see if a write will not block.
 * @param mixed $except - The sockets listed in the except array will be
 *   watched for exceptions.
 * @param mixed $vtv_sec - The tv_sec and tv_usec together form the timeout
 *   parameter. The timeout is an upper bound on the amount of time elapsed
 *   before socket_select() return. tv_sec may be zero , causing socket_select()
 *   to return immediately. This is useful for polling. If tv_sec is NULL (no
 *   timeout), socket_select() can block indefinitely.
 * @param int $tv_usec
 *
 * @return mixed - On success socket_select() returns the number of socket
 *   resources contained in the modified arrays, which may be zero if the
 *   timeout expires before anything interesting happens. On error FALSE is
 *   returned. The error code can be retrieved with socket_last_error().  Be
 *   sure to use the === operator when checking for an error. Since the
 *   socket_select() may return 0 the comparison with == would evaluate to TRUE:
 *   Example #2 Understanding socket_select()'s result
 *
 */
<<__Native>>
function socket_select(inout mixed $read,
                       inout mixed $write,
                       inout mixed $except,
                       mixed $vtv_sec,
                       int $tv_usec = 0)[leak_safe]: mixed;

<<__Native>>
function socket_server(string $hostname,
                       int $port,
                       <<__OutOnly>>
                       inout mixed $errnum,
                       <<__OutOnly>>
                       inout mixed $errstr): mixed;

/**
 * After the socket socket has been created using socket_create(), bound to a
 *   name with socket_bind(), and told to listen for connections with
 *   socket_listen(), this function will accept incoming connections on that
 *   socket. Once a successful connection is made, a new socket resource is
 *   returned, which may be used for communication. If there are multiple
 *   connections queued on the socket, the first will be used. If there are no
 *   pending connections, socket_accept() will block until a connection becomes
 *   present. If socket has been made non-blocking using socket_set_blocking()
 *   or socket_set_nonblock(), FALSE will be returned.  The socket resource
 *   returned by socket_accept() may not be used to accept new connections. The
 *   original listening socket socket, however, remains open and may be reused.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create().
 *
 * @return mixed - Returns a new socket resource on success, or FALSE on
 *   error. The actual error code can be retrieved by calling
 *   socket_last_error(). This error code may be passed to socket_strerror() to
 *   get a textual explanation of the error.
 *
 */
<<__Native>>
function socket_accept(resource $socket): mixed;

/**
 * The function socket_read() reads from the socket resource socket created by
 *   the socket_create() or socket_accept() functions.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create() or socket_accept().
 * @param int $length - The maximum number of bytes read is specified by the
 *   length parameter. Otherwise you can use \r, \n, or \0 to end reading
 *   (depending on the type parameter, see below).
 * @param int $type - Optional type parameter is a named constant:
 *   PHP_BINARY_READ (Default) - use the system recv() function. Safe for
 *   reading binary data. PHP_NORMAL_READ - reading stops at \n or \r.
 *
 * @return mixed - socket_read() returns the data as a string on success, or
 *   FALSE on error (including if the remote host has closed the connection).
 *   The error code can be retrieved with socket_last_error(). This code may be
 *   passed to socket_strerror() to get a textual representation of the error.
 *   socket_read() returns a zero length string ("") when there is no more data
 *   to read.
 *
 */
<<__Native>>
function socket_read(resource $socket, int $length, int $type = 0): mixed;

/**
 * The function socket_write() writes to the socket from the given buffer.
 *
 * @param resource $socket
 * @param string $buffer - The buffer to be written.
 * @param int $length - The optional parameter length can specify an alternate
 *   length of bytes written to the socket. If this length is greater than the
 *   buffer length, it is silently truncated to the length of the buffer.
 *
 * @return mixed - Returns the number of bytes successfully written to the
 *   socket or FALSE on failure. The error code can be retrieved with
 *   socket_last_error(). This code may be passed to socket_strerror() to get a
 *   textual explanation of the error.  It is perfectly valid for socket_write()
 *   to return zero which means no bytes have been written. Be sure to use the
 *   === operator to check for FALSE in case of an error.
 *
 */
<<__Native>>
function socket_write(resource $socket, string $buffer, int $length = 0): mixed;

/**
 * The function socket_send() sends len bytes to the socket socket from buf.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create() or socket_accept().
 * @param string $buf - A buffer containing the data that will be sent to the
 *   remote host.
 * @param int $len - The number of bytes that will be sent to the remote host
 *   from buf.
 * @param int $flags - The value of flags can be any combination of the
 *   following flags, joined with the binary OR (|) operator. Possible values
 *   for flags MSG_OOB Send OOB (out-of-band) data. MSG_EOR Indicate a record
 *   mark. The sent data completes the record. MSG_EOF Close the sender side of
 *   the socket and include an appropriate notification of this at the end of
 *   the sent data. The sent data completes the transaction. MSG_DONTROUTE
 *   Bypass routing, use direct interface.
 *
 * @return mixed - socket_send() returns the number of bytes sent, or FALSE on
 *   error.
 *
 */
<<__Native>>
function socket_send(resource $socket,
                     string $buf,
                     int $len,
                     int $flags): mixed;

/**
 * The function socket_sendto() sends len bytes from buf through the socket
 *   socket to the port at the address addr.
 *
 * @param resource $socket - A valid socket resource created using
 *   socket_create().
 * @param string $buf - The sent data will be taken from buffer buf.
 * @param int $len - len bytes from buf will be sent.
 * @param int $flags - The value of flags can be any combination of the
 *   following flags, joined with the binary OR (|) operator. Possible values
 *   for flags MSG_OOB Send OOB (out-of-band) data. MSG_EOR Indicate a record
 *   mark. The sent data completes the record. MSG_EOF Close the sender side of
 *   the socket and include an appropriate notification of this at the end of
 *   the sent data. The sent data completes the transaction. MSG_DONTROUTE
 *   Bypass routing, use direct interface.
 * @param string $addr - IP address of the remote host.
 * @param int $port - port is the remote port number at which the data will be
 *   sent.
 *
 * @return mixed - socket_sendto() returns the number of bytes sent to the
 *   remote host, or FALSE if an error occurred.
 *
 */
<<__Native>>
function socket_sendto(resource $socket,
                       string $buf,
                       int $len,
                       int $flags,
                       string $addr,
                       int $port = -1): mixed;

/**
 * The socket_recv() function receives len bytes of data in buf from socket.
 *   socket_recv() can be used to gather data from connected sockets.
 *   Additionally, one or more flags can be specified to modify the behaviour of
 *   the function.  buf is passed by reference, so it must be specified as a
 *   variable in the argument list. Data read from socket by socket_recv() will
 *   be returned in buf.
 *
 * @param resource $socket - The socket must be a socket resource previously
 *   created by socket_create().
 * @param mixed $buf - The data received will be fetched to the variable
 *   specified with buf. If an error occurs, if the connection is reset, or if
 *   no data is available, buf will be set to NULL.
 * @param int $len - Up to len bytes will be fetched from remote host.
 * @param int $flags - The value of flags can be any combination of the
 *   following flags, joined with the binary OR (|) operator. Possible values
 *   for flags Flag Description MSG_OOB Process out-of-band data. MSG_PEEK
 *   Receive data from the beginning of the receive queue without removing it
 *   from the queue. MSG_WAITALL Block until at least len are received. However,
 *   if a signal is caught or the remote host disconnects, the function may
 *   return less data. MSG_DONTWAIT With this flag set, the function returns
 *   even if it would normally have blocked.
 *
 * @return mixed - socket_recv() returns the number of bytes received, or
 *   FALSE if there was an error. The actual error code can be retrieved by
 *   calling socket_last_error(). This error code may be passed to
 *   socket_strerror() to get a textual explanation of the error.
 *
 */
<<__Native>>
function socket_recv(resource $socket,
                     <<__OutOnly>>
                     inout mixed $buf,
                     int $len,
                     int $flags): mixed;

/**
 * The socket_recvfrom() function receives len bytes of data in buf from name
 *   on port port (if the socket is not of type AF_UNIX) using socket.
 *   socket_recvfrom() can be used to gather data from both connected and
 *   unconnected sockets. Additionally, one or more flags can be specified to
 *   modify the behaviour of the function.  The name and port must be passed by
 *   reference. If the socket is not connection-oriented, name will be set to
 *   the internet protocol address of the remote host or the path to the UNIX
 *   socket. If the socket is connection-oriented, name is NULL. Additionally,
 *   the port will contain the port of the remote host in the case of an
 *   unconnected AF_INET or AF_INET6 socket.
 *
 * @param resource $socket - The socket must be a socket resource previously
 *   created by socket_create().
 * @param mixed $buf - The data received will be fetched to the variable
 *   specified with buf.
 * @param int $len - Up to len bytes will be fetched from remote host.
 * @param int $flags - The value of flags can be any combination of the
 *   following flags, joined with the binary OR (|) operator. Possible values
 *   for flags Flag Description MSG_OOB Process out-of-band data. MSG_PEEK
 *   Receive data from the beginning of the receive queue without removing it
 *   from the queue. MSG_WAITALL Block until at least len are received. However,
 *   if a signal is caught or the remote host disconnects, the function may
 *   return less data. MSG_DONTWAIT With this flag set, the function returns
 *   even if it would normally have blocked.
 * @param mixed $name - If the socket is of the type AF_UNIX type, name is the
 *   path to the file. Else, for unconnected sockets, name is the IP address of,
 *   the remote host, or NULL if the socket is connection-oriented.
 * @param mixed $port - This argument only applies to AF_INET and AF_INET6
 *   sockets, and specifies the remote port from which the data is received. If
 *   the socket is connection-oriented, port will be NULL.
 *
 * @return mixed - socket_recvfrom() returns the number of bytes received, or
 *   FALSE if there was an error. The actual error code can be retrieved by
 *   calling socket_last_error(). This error code may be passed to
 *   socket_strerror() to get a textual explanation of the error.
 *
 */
<<__Native>>
function socket_recvfrom(resource $socket,
                         <<__OutOnly>>
                         inout mixed $buf,
                         int $len,
                         int $flags,
                         <<__OutOnly>>
                         inout mixed $name,
                         <<__OutOnly>>
                         inout mixed $port): mixed;

/**
 * The socket_shutdown() function allows you to stop incoming, outgoing or all
 *   data (the default) from being sent through the socket
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create().
 * @param int $how - The value of how can be one of the following: possible
 *   values for how 0 Shutdown socket reading 1 Shutdown socket writing 2
 *   Shutdown socket reading and writing
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function socket_shutdown(resource $socket, int $how = 0): bool;

/**
 * socket_close() closes the socket resource given by socket. This function is
 *   specific to sockets and cannot be used on any other type of resources.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create() or socket_accept().
 *
 */
<<__Native>>
function socket_close(resource $socket)[write_props]: void;

/**
 * socket_strerror() takes as its errno parameter a socket error code as
 *   returned by socket_last_error() and returns the corresponding explanatory
 *   text.  Although the error messages generated by the socket extension are in
 *   English, the system messages retrieved with this function will appear
 *   depending on the current locale (LC_MESSAGES).
 *
 * @param int $errnum - A valid socket error number, likely produced by
 *   socket_last_error().
 *
 * @return string - Returns the error message associated with the errno
 *   parameter.
 *
 */
<<__Native>>
function socket_strerror(int $errnum)[]: string;

/**
 * If a socket resource is passed to this function, the last error which
 *   occurred on this particular socket is returned. If the socket resource is
 *   omitted, the error code of the last failed socket function is returned. The
 *   latter is particularly helpful for functions like socket_create() which
 *   don't return a socket on failure and socket_select() which can fail for
 *   reasons not directly tied to a particular socket. The error code is
 *   suitable to be fed to socket_strerror() which returns a string describing
 *   the given error code.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create().
 *
 * @return int - This function returns a socket error code.
 *
 */
<<__Native>>
function socket_last_error(?resource $socket = null)[read_globals]: int;

/**
 * This function clears the error code on the given socket or the global last
 *   socket error if no socket is specified.  This function allows explicitly
 *   resetting the error code value either of a socket or of the extension
 *   global last error code. This may be useful to detect within a part of the
 *   application if an error occurred or not.
 *
 * @param resource $socket - A valid socket resource created with
 *   socket_create().
 *
 */
<<__Native>>
function socket_clear_error(?resource $socket = null): void;

<<__Native>>
function getaddrinfo(string $host,
                     string $port,
                     int $family = 0,
                     int $socktype = 0,
                     int $protocol = 0,
                     int $flags = 0): mixed;
