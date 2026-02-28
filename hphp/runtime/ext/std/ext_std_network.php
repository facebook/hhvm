<?hh

namespace {

/**
 * Check DNS records corresponding to a given Internet host name or IP address
 *
 * @param string $host - host may either be the IP address in dotted-quad
 *   notation or the host name.
 * @param string $type - type may be any one of: A, MX, NS, SOA, PTR,
 *   CNAME, AAAA, A6, SRV, NAPTR, TXT or ANY.
 *
 * @return bool - Returns TRUE if any records are found; returns FALSE if
 *   no records were found or if an error occurred.
 */
<<__Native>>
function checkdnsrr(string $host,
                    string $type = "MX")[defaults]: bool;

/**
 * Close connection to system logger
 *
 * @return bool -
 */
<<__Native>>
function closelog(): bool;

/**
 * Initializes all syslog related variables
 *
 * @return void - Syslog variables    Variable Constant equal Meaning
 *   Notes     $LOG_EMERG LOG_EMERG System is unusable    $LOG_ALERT
 *   LOG_ALERT Immediate action required    $LOG_CRIT LOG_CRIT Critical
 *   conditions    $LOG_ERR LOG_ERR     $LOG_WARNING LOG_WARNING
 *   $LOG_NOTICE LOG_NOTICE     $LOG_INFO LOG_INFO     $LOG_DEBUG LOG_DEBUG
 *       $LOG_KERN LOG_KERN     $LOG_USER LOG_USER Genetic user level
 *   $LOG_MAIL LOG_MAIL Log to email    $LOG_DAEMON LOG_DAEMON Other system
 *   daemons    $LOG_AUTH LOG_AUTH     $LOG_SYSLOG LOG_SYSLOG  Not
 *   available on Netware   $LOG_LPR LOG_LPR     $LOG_NEWS LOG_NEWS Usenet
 *   new Not available on HP-UX   $LOG_CRON LOG_CRON  Not available on all
 *   platforms   $LOG_AUTHPRIV LOG_AUTHPRIV  Not available on AIX
 *   $LOG_LOCAL0 LOG_LOCAL0  Not available on Windows and Netware
 *   $LOG_LOCAL1 LOG_LOCAL1  Not available on Windows and Netware
 *   $LOG_LOCAL2 LOG_LOCAL2  Not available on Windows and Netware
 *   $LOG_LOCAL3 LOG_LOCAL3  Not available on Windows and Netware
 *   $LOG_LOCAL4 LOG_LOCAL4  Not available on Windows and Netware
 *   $LOG_LOCAL5 LOG_LOCAL5  Not available on Windows and Netware
 *   $LOG_LOCAL6 LOG_LOCAL6  Not available on Windows and Netware
 *   $LOG_LOCAL7 LOG_LOCAL7  Not available on Windows and Netware
 *   $LOG_PID LOG_PID     $LOG_CONS LOG_CONS     $LOG_ODELAY LOG_ODELAY
 *   $LOG_NDELAY LOG_NDELAY     $LOG_NOWAIT LOG_NOWAIT  Not available on
 *   BeOS   $LOG_PERROR LOG_PERROR  Not available on AIX
 */
function define_syslog_variables(): void {
  // do nothing, since all variables are defined as constants already
}

/**
 * Alias of checkdnsrr()
 */
function dns_check_record(string $host, string $type = 'MX')[defaults]: bool {
  return checkdnsrr($host, $type);
}

/**
 * Alias of getmxrr
 */
function dns_get_mx(string $host, inout mixed $mxhosts, inout mixed $weight)[defaults]: bool {
  $ret = getmxrr($host, inout $mxhosts, inout $weight);
  return $ret;
}

/**
 * Fetch DNS Resource Records associated with a hostname
 *
 * @param string $hostname - hostname should be a valid DNS hostname such
 *   as "www.example.com". Reverse lookups can be generated using
 *   in-addr.arpa notation, but gethostbyaddr() is more suitable for the
 *   majority of reverse lookups.    Per DNS standards, email addresses are
 *   given in user.host format (for example: hostmaster.example.com as
 *   opposed to hostmaster@example.com), be sure to check this value and
 *   modify if necessary before using it with a functions such as mail().
 * @param int $type - By default, dns_get_record() will search for any
 *   resource records associated with hostname. To limit the query, specify
 *   the optional type parameter. May be any one of the following: DNS_A,
 *   DNS_CNAME, DNS_HINFO, DNS_CAA, DNS_MX, DNS_NS, DNS_PTR, DNS_SOA, DNS_TXT,
 *   DNS_AAAA, DNS_SRV, DNS_NAPTR, DNS_A6, DNS_ALL or DNS_ANY.    Because
 *   of eccentricities in the performance of libresolv between platforms,
 *   DNS_ANY will not always return every record, the slower DNS_ALL will
 *   collect all records more reliably.
 * @param array $authns - Passed by reference and, if given, will be
 *   populated with Resource Records for the Authoritative Name Servers.
 * @param array $addtl - Passed by reference and, if given, will be
 *   populated with any Additional Records.
 *
 * @return array - This function returns an array of associative arrays,
 *   . Each associative array contains at minimum the following keys:
 *   Basic DNS attributes    Attribute Meaning     host  The record in the
 *   DNS namespace to which the rest of the associated data refers.
 *   class  dns_get_record() only returns Internet class records and as
 *   such this parameter will always return IN.    type  String containing
 *   the record type. Additional attributes will also be contained in the
 *   resulting array dependant on the value of type. See table below.
 *   ttl  "Time To Live" remaining for this record. This will not equal the
 *   record's original ttl, but will rather equal the original ttl minus
 *   whatever length of time has passed since the authoritative name server
 *   was queried.         Other keys in associative arrays dependant on
 *   'type'    Type Extra Columns     A  ip: An IPv4 addresses in dotted
 *   decimal notation.    MX  pri: Priority of mail exchanger. Lower
 *   numbers indicate greater priority. target: FQDN of the mail exchanger.
 *   See also dns_get_mx().    CNAME  target: FQDN of location in DNS
 *   namespace to which the record is aliased.    NS  target: FQDN of the
 *   name server which is authoritative for this hostname.    PTR  target:
 *   Location within the DNS namespace to which this record points.    TXT
 *   txt: Arbitrary string data associated with this record.    HINFO  cpu:
 *   IANA number designating the CPU of the machine referenced by this
 *   record. os: IANA number designating the Operating System on the
 *   machine referenced by this record. See IANA's Operating System Names
 *   for the meaning of these values.    SOA  mname: FQDN of the machine
 *   from which the resource records originated. rname: Email address of
 *   the administrative contain for this domain. serial: Serial # of this
 *   revision of the requested domain. refresh: Refresh interval (seconds)
 *   secondary name servers should use when updating remote copies of this
 *   domain. retry: Length of time (seconds) to wait after a failed refresh
 *   before making a second attempt. expire: Maximum length of time
 *   (seconds) a secondary DNS server should retain remote copies of the
 *   zone data without a successful refresh before discarding. minimum-ttl:
 *   Minimum length of time (seconds) a client can continue to use a DNS
 *   resolution before it should request a new resolution from the server.
 *   Can be overridden by individual resource records.    AAAA  ipv6: IPv6
 *   address    A6(PHP = 5.1.0)  masklen: Length (in bits) to inherit from
 *   the target specified by chain. ipv6: Address for this specific record
 *   to merge with chain. chain: Parent record to merge with ipv6 data.
 *   SRV  pri: (Priority) lowest priorities should be used first. weight:
 *   Ranking to weight which of commonly prioritized targets should be
 *   chosen at random. target and port: hostname and port where the
 *   requested service can be found. For additional information see: RFC
 *   2782    NAPTR  order and pref: Equivalent to pri and weight above.
 *   flags, services, regex, and replacement: Parameters as defined by RFC
 *   2915.
 */
<<__Native>>
function dns_get_record(string $hostname,
                        int $type,
                        <<__OutOnly>>
                        inout mixed $authns,
                        <<__OutOnly>>
                        inout mixed $addtl)[defaults]: mixed;

/**
 * Open Internet or Unix domain socket connection
 *
 * @param string $hostname - If OpenSSL support is installed, you may
 *   prefix the hostname with either ssl:// or tls:// to use an SSL or TLS
 *   client connection over TCP/IP to connect to the remote host.
 * @param int $port - The port number. This can be skipped with -1
 * for transports that do not use ports, such as unix://.
 * @param int $errno - If provided, holds the system level error number
 *   that occurred in the system-level connect() call.   If the value
 *   returned in errno is 0 and the function returned FALSE, it is an
 *   indication that the error occurred before the connect() call. This is
 *   most likely due to a problem initializing the socket.
 * @param string $errstr - The error message as a string.
 * @param float $timeout - The connection timeout, in seconds.    If you
 *   need to set a timeout for reading/writing data over the socket, use
 *   stream_set_timeout(), as the timeout parameter to fsockopen() only
 *   applies while connecting the socket.
 *
 * @return resource - fsockopen() returns a file pointer which may be
 *   used together with the other file functions (such as fgets(),
 *   fgetss(), fwrite(), fclose(), and feof()). If the call fails, it will
 *   return FALSE
 */
<<__Native>>
function fsockopen(string $hostname,
                   int $port,
                   <<__OutOnly>>
                   inout mixed $errno,
                   <<__OutOnly>>
                   inout mixed $errstr,
                   float $timeout = -1.0): mixed;

/**
 * get_http_request_size() will return the size of the http request.
 *
 * @return int - Returns the size of the http request.
 */
<<__Native>>
function get_http_request_size()[read_globals]: int;

/**
 * Get the Internet host name corresponding to a given IP address
 *
 *
 * @param string $ip_address - The host IP address.
 *
 * @return string - Returns the host name on success, the unmodified
 *   ip_address on failure, or FALSE on malformed input.
 */
<<__Native>>
function gethostbyaddr(string $ip_address): mixed;

/**
 * Get the IPv4 address corresponding to a given Internet host name
 *
 *
 * @param string $hostname - The host name.
 *
 * @return string - Returns the IPv4 address or a string containing the
 *   unmodified hostname on failure.
 */
<<__Native>>
function gethostbyname(string $hostname): string;

/**
 * Get a list of IPv4 addresses corresponding to a given Internet host
 *    name
 *
 *
 * @param string $hostname - The host name.
 *
 * @return array - Returns an array of IPv4 addresses or FALSE if
 *   hostname could not be resolved.
 */
<<__Native>>
function gethostbynamel(string $hostname): mixed;

/**
 * Gets the host name
 *
 * @return string - Returns a string with the hostname on success,
 *   otherwise FALSE is returned.
 */
<<__Native>>
function gethostname()[]: mixed;

/**
 * Get MX records corresponding to a given Internet host name
 *
 * @param string $hostname - The Internet host name.
 * @param array $mxhosts - A list of the MX records found is placed into
 *   the array mxhosts.
 * @param array $weight - If the weight array is given, it will be filled
 *   with the weight information gathered.
 *
 * @return bool - Returns TRUE if any records are found; returns FALSE if
 *   no records were found or if an error occurred.
 */
<<__Native>>
function getmxrr(string $hostname,
                 <<__OutOnly>>
                 inout mixed $mxhosts,
                 <<__OutOnly>>
                 inout mixed $weight)[defaults]: bool;

/**
 * Get protocol number associated with protocol name
 *
 * @param string $name - The protocol name.
 *
 * @return int - Returns the protocol number, .
 */
<<__Native>>
function getprotobyname(string $name): mixed;

/**
 * Get protocol name associated with protocol number
 *
 * @param int $number - The protocol number.
 *
 * @return string - Returns the protocol name as a string, .
 */
<<__Native>>
function getprotobynumber(int $number): mixed;

/**
 * Get port number associated with an Internet service and protocol
 *
 * @param string $service - The Internet service name, as a string.
 * @param string $protocol - protocol is either "tcp" or "udp" (in
 *   lowercase).
 *
 * @return int - Returns the port number, or FALSE if service or protocol
 *   is not found.
 */
<<__Native>>
function getservbyname(string $service,
                       string $protocol): mixed;

/**
 * Get Internet service which corresponds to port and protocol
 *
 * @param int $port - The port number.
 * @param string $protocol - protocol is either "tcp" or "udp" (in
 *   lowercase).
 *
 * @return string - Returns the Internet service name as a string.
 */
<<__Native>>
function getservbyport(int $port,
                       string $protocol): mixed;

/**
 * Call a header function
 *
 * @param callable $callback -
 *
 * @return bool -
 */
<<__Native("NoRecording")>>
function header_register_callback(mixed $callback)[globals]: mixed;

/**
 * Remove previously set headers
 *
 * @param string $name - The header name to be removed.    This parameter
 *   is case-insensitive.
 *
 * @return void -
 */
<<__Native>>
function header_remove(?string $name = null)[globals]: void;

/**
 * Send a raw HTTP header
 *
 * @param string $string - The header string.   There are two
 *   special-case header calls. The first is a header that starts with the
 *   string "HTTP/" (case is not significant), which will be used to figure
 *   out the HTTP status code to send. For example, if you have configured
 *   Apache to use a PHP script to handle requests for missing files (using
 *   the ErrorDocument directive), you may want to make sure that your
 *   script generates the proper status code.          The second special
 *   case is the "Location:" header. Not only does it send this header back
 *   to the browser, but it also returns a REDIRECT (302) status code to
 *   the browser unless the 201 or a 3xx status code has already been set.
 * @param bool $replace - The optional replace parameter indicates
 *   whether the header should replace a previous similar header, or add a
 *   second header of the same type. By default it will replace, but if you
 *   pass in FALSE as the second argument you can force multiple headers of
 *   the same type. For example:
 * @param int $http_response_code - Forces the HTTP response code to the
 *   specified value. Note that this parameter only has an effect if the
 *   string is not empty.
 *
 * @return void -
 */
<<__Native>>
function header(string $string,
                bool $replace = true,
                int $http_response_code = 0)[globals]: void;

/**
 * Returns a list of response headers sent (or ready to send)
 *
 * @return array - Returns a numerically indexed array of headers.
 */
<<__Native>>
function headers_list()[read_globals]: varray<string>;

/**
 * Checks if or where headers have been sent
 *
 * @param string $file - If the optional file and line parameters are
 *   set, headers_sent() will put the PHP source file name and line number
 *   where output started in the file and line variables.
 * @param int $line - The line number where the output started.
 *
 * @return bool - headers_sent() will return FALSE if no HTTP headers
 *   have already been sent or TRUE otherwise.
 */
<<__Native>>
function headers_sent()[read_globals]: bool;

<<__Native>>
function headers_sent_with_file_line(
  <<__OutOnly>>
  inout mixed $file,
  <<__OutOnly>>
  inout mixed $line)[read_globals]: bool;

/**
 * Get or Set the HTTP response code
 *
 * @param int $response_code - The optional response_code will set the
 *   response code.
 *
 * @return int - The current response code. By default the return value
 *   is int(200).
 */
<<__Native>>
function http_response_code(int $response_code = 0)[globals]: mixed;

/**
 * Converts a packed internet address to a human readable representation
 *
 * @param string $in_addr - A 32bit IPv4, or 128bit IPv6 address.
 *
 * @return string - Returns a string representation of the address.
 */
<<__IsFoldable, __Native>>
function inet_ntop(string $in_addr)[]: mixed;

/**
 * Converts a packed internet address to a human readable representation
 *
 * @param string $in_addr - A 32bit IPv4, or 128bit IPv6 address.
 * @return null|string - Returns a string representation of the address.
 */
<<__IsFoldable, __Native>>
function inet_ntop_nullable(string $in_addr)[]: ?string;

/**
 * inet_ntop() using the folly library. Used for performance benchmarking.
 * @param string $in_addr - A 32bit IPv4, or 128bit IPv6 address.
 * @return null|string - Returns a string representation of the address.
 */
<<__IsFoldable, __Native>>
function inet_ntop_folly(string $in_addr)[]: ?string;

/**
 * Converts a human readable IP address to its packed in_addr representation
 *
 * @param string $address - A human readable IPv4 or IPv6 address.
 *
 * @return string - Returns the in_addr representation of the given
 *   address, or FALSE if a syntactically invalid address is given (for
 *   example, an IPv4 address without dots or an IPv6 address without
 *   colons).
 */
<<__IsFoldable, __Native>>
function inet_pton(string $address)[]: mixed;

/**
 * Converts a string containing an (IPv4) Internet Protocol dotted address
 * into a proper address
 *
 * @param string $ip_address - A standard format address.
 *
 * @return int - Returns the IPv4 address or FALSE if ip_address is
 *   invalid.
 */
<<__IsFoldable, __Native>>
function ip2long(string $ip_address)[]: mixed;

/**
 * Converts an (IPv4) Internet network address into a string in Internet
 * standard dotted format
 *
 * @param string $proper_address - A proper address representation.
 *
 * @return string - Returns the Internet IP address as a string.
 */
<<__IsFoldable, __Native>>
function long2ip(string $proper_address)[]: string;

/**
 * Open connection to system logger
 *
 * @param string $ident - The string ident is added to each message.
 * @param int $option - The option argument is used to indicate what
 *   logging options will be used when generating a log message.  openlog()
 *   Options    Constant Description     LOG_CONS  if there is an error
 *   while sending data to the system logger, write directly to the system
 *   console    LOG_NDELAY  open the connection to the logger immediately
 *    LOG_ODELAY  (default) delay opening the connection until the first
 *   message is logged    LOG_PERROR print log message also to standard
 *   error   LOG_PID include PID with each message     You can use one or
 *   more of this options. When using multiple options you need to OR them,
 *   i.e. to open the connection immediately, write to the console and
 *   include the PID in each message, you will use: LOG_CONS | LOG_NDELAY |
 *   LOG_PID
 * @param int $facility - The facility argument is used to specify what
 *   type of program is logging the message. This allows you to specify (in
 *   your machine's syslog configuration) how messages coming from
 *   different facilities will be handled.  openlog() Facilities
 *   Constant Description     LOG_AUTH  security/authorization messages
 *   (use LOG_AUTHPRIV instead in systems where that constant is defined)
 *    LOG_AUTHPRIV security/authorization messages (private)   LOG_CRON
 *   clock daemon (cron and at)   LOG_DAEMON other system daemons
 *   LOG_KERN kernel messages   LOG_LOCAL0 ... LOG_LOCAL7 reserved for
 *   local use, these are not available in Windows   LOG_LPR line printer
 *   subsystem   LOG_MAIL mail subsystem   LOG_NEWS USENET news subsystem
 *   LOG_SYSLOG messages generated internally by syslogd   LOG_USER generic
 *   user-level messages   LOG_UUCP UUCP subsystem        LOG_USER is the
 *   only valid log type under Windows operating systems
 *
 * @return bool -
 */
<<__Native>>
function openlog(string $ident,
                 int $option,
                 int $facility): bool;

/**
 * Open persistent Internet or Unix domain socket connection
 *
 * @param string $hostname -
 * @param int $port -
 * @param int $errno -
 * @param string $errstr -
 * @param float $timeout -
 *
 * @return resource -
 */
<<__Native>>
function pfsockopen(string $hostname,
                    int $port,
                    <<__OutOnly>>
                    inout mixed $errno,
                    <<__OutOnly>>
                    inout mixed $errstr,
                    float $timeout = -1.0): mixed;

/**
 * Send a cookie
 *
 * @param string $name -
 * @param string $value -
 * @param int $expire -
 * @param string $path -
 * @param string $domain -
 * @param bool $secure -
 * @param bool $httponly -
 *
 * @return bool - If output exists prior to calling this function,
 *   setcookie() will fail and return FALSE. If setcookie() successfully
 *   runs, it will return TRUE. This does not indicate whether the user
 *   accepted the cookie.
 */
<<__Native>>
function setcookie(string $name,
                   string $value = '',
                   int $expire = 0,
                   string $path = '',
                   string $domain = '',
                   bool $secure = false,
                   bool $httponly = false): bool;

/**
 * Send a cookie without urlencoding the cookie value
 *
 * @param string $name -
 * @param string $value -
 * @param int $expire -
 * @param string $path -
 * @param string $domain -
 * @param bool $secure -
 * @param bool $httponly -
 *
 * @return bool -
 */
<<__Native>>
function setrawcookie(string $name,
                      string $value = '',
                      int $expire = 0,
                      string $path = '',
                      string $domain = '',
                      bool $secure = false,
                      bool $httponly = false): bool;

function socket_get_status(resource $stream): mixed {
  return stream_get_meta_data($stream);
}

function socket_set_blocking(resource $stream, bool $mode): bool {
  return stream_set_blocking($stream, $mode);
}

function socket_set_timeout(resource $stream, int $secs, int $msecs = 0): bool {
  return stream_set_timeout($stream, $secs, $msecs);
}

/**
 * Generate a system log message
 *
 * @param int $priority - priority is a combination of the facility and
 *   the level. Possible values are:  syslog() Priorities (in descending
 *   order)    Constant Description     LOG_EMERG system is unusable
 *   LOG_ALERT action must be taken immediately   LOG_CRIT critical
 *   conditions   LOG_ERR error conditions   LOG_WARNING warning conditions
 *     LOG_NOTICE normal, but significant, condition   LOG_INFO
 *   informational message   LOG_DEBUG debug-level message
 * @param string $message - The message to send, except that the two
 *   characters %m will be replaced by the error message string (strerror)
 *   corresponding to the present value of errno.
 *
 * @return bool -
 */
<<__Native>>
function syslog(int $priority,
                string $message): bool;

}

namespace HH {

/**
 * Given a cookie header value, parse it in the same style that the $_COOKIE
 * global uses.
 *
 * @param string $header_value - Cookie header value.
 *
 * @return dict<arraykey, mixed> - dict of cookie names and values. Similarly to
 *   $_COOKIE, cookie names like 'foo[0]=bar; foo[1]=baz' are returned as
 *   `dict['foo' => vec['bar', 'baz']]`.
 */
<<__Native>>
function parse_cookies(string $header_value)[]: dict<arraykey, mixed>;

}
