<?hh

/**
 * Initiate an asynchronous (non-blocking) connection to the specified MySQL
 * server.
 *
 * @param string $server   - The MySQL server. It can also include a port
 *                           number. e.g. "hostname:port" or a path to a local
 *                           socket e.g. ":/path/to/socket" for the localhost.
 *
 *                           If the PHP directive mysql.default_host is
 *                           undefined (default), then the default value is
 *                           'localhost:3306'. In SQL safe mode, this parameter
 *                           is ignored and value 'localhost:3306' is always
 *                           used.
 * @param string $username - The username. Default value is defined by
 *                           mysql.default_user. In SQL safe mode, this
 *                           parameter is ignored and the name of the user that
 *                           owns the server process is used.
 * @param string $password - The password. Default value is defined by
 *                           mysql.default_password. In SQL safe mode, this
 *                           parameter is ignored and empty password is used.
 * @param string $database - The name of the database that will be selected.
 *
 * @return mixed - Initiate an asynchronous mysql connection.
 */
<<__Native, __HipHopSpecific>>
function mysql_async_connect_start(string $server = '', string $username = '',
                                   string $password = '', string $database = ''
                                   ): mixed;

/**
 * A nonblocking test whether a connection has completed, or errored out.
 *
 * @param resource $link_identifier - The MySQL connection. If the link
 *                                    identifier is not specified, the last link
 *                                    opened by mysql_connect() is assumed. If
 *                                    no such link is found, it will try to
 *                                    create one as if mysql_connect() was
 *                                    called with no arguments. If no connection
 *                                    is found or established, an E_WARNING
 *                                    level error is generated.
 *
 * @return bool - Has the connection finished (either successfully or with
 *                error).
 */
<<__Native, __HipHopSpecific>>
function mysql_async_connect_completed(?resource $link_identifier = NULL): bool;

/**
 * Initiate a nonblocking query on a given connection.
 *
 * @param string $query             - An SQL query
 *
 *                                    The query string should not end with a
 *                                    semicolon. Data inside the query should be
 *                                    properly escaped.
 * @param resource $link_identifier - The MySQL connection. If the link
 *                                    identifier is not specified, the last link
 *                                    opened by mysql_connect() is assumed. If
 *                                    no such link is found, it will try to
 *                                    create one as if mysql_connect() was
 *                                    called with no arguments. If no connection
 *                                    is found or established, an E_WARNING
 *                                    level error is generated.
 *
 * @return bool - TRUE if the query can properly be prepared and queued on the
 *                network.
 */
<<__Native, __HipHopSpecific>>
function mysql_async_query_start(string $query,
                                 ?resource $link_identifier = NULL): bool;

/**
 * Fetch a result object, if available, containing some rows of the nonblocking
 * query.
 *
 * @param resource $link_identifier - The MySQL connection. If the link
 *                                    identifier is not specified, the last link
 *                                    opened by mysql_connect() is assumed. If
 *                                    no such link is found, it will try to
 *                                    create one as if mysql_connect() was
 *                                    called with no arguments. If no connection
 *                                    is found or established, an E_WARNING
 *                                    level error is generated.
 *
 * @return resource - A mysql result object, or null if one isn't ready yet.
 */
<<__Native, __HipHopSpecific>>
function mysql_async_query_result(?resource $link_identifier = NULL): ?resource;

/**
 * Perform a nonblocking test whether an asynchronous query has completed.
 *
 * @param resource $result - The mysql result object from
 *                           mysql_async_query_result.
 *
 * @return bool - True if the the query has completed (i.e., either all rows
 *                have been returned or an error occurred).
 */
<<__Native, __HipHopSpecific>>
function mysql_async_query_completed(resource $result): bool;

/**
 * Returns an array that corresponds to the fetched row, if available.
 * Nonblocking.
 *
 * @param resource $result - resource that is being evaluated. This result comes
 *                           from a call to mysql_async_query_result().
 * @param int $result_type - The type of array that is to be fetched. It's a
 *                           constant and can take the following values:
 *                           MYSQL_ASSOC, MYSQL_NUM, and MYSQL_BOTH. The default
 *                           is MYSQL_ASSOC
 *
 * @return mixed - Returns an array of strings that corresponds to the fetched
 *                 row, or FALSE if there are no rows currently available. The
 *                 type of returned array depends on how result_type is defined.
 *                 By using MYSQL_BOTH, you'll get an array with both
 *                 associative and number indices. Using MYSQL_ASSOC (the
 *                 default), you only get associative indices (as
 *                 mysql_fetch_assoc() works), using MYSQL_NUM, you only get
 *                 number indices (as mysql_fetch_row() works).
 *
 *                 If two or more columns of the result have the same field
 *                 names, the last column will take precedence. To access the
 *                 other column(s) of the same name, you must use the numeric
 *                 index of the column or make an alias for the column. For
 *                 aliased columns, you cannot access the contents with the
 *                 original column name.
 */
<<__Native, __HipHopSpecific>>
function mysql_async_fetch_array(resource $result,
                                 int $result_type = MYSQL_BOTH): mixed;

/**
 * Block on one or more asynchronous operations, or until the specified timeout
 * has occurred. Returns values from the 'items' parameter when they become
 * actionable. Entries are returned as soon as they are actionable (i.e., it
 * does not wait for the entire timeout before returning results).
 *
 * @param array $items   - An array of arrays. These arrays contain a MySQL link
 *                         identifier in the 0'th position, and any other values
 *                         in the remainder of the array. Items from this
 *                         parameter are returned unmodified as the result set of
 *                         actionable entries.
 * @param float $timeout - Time, in seconds, to wait for actionable events.
 *                         Subsecond accuracy is supported.
 *
 * @return mixed - Returns input entries that are now ready for action (such as
 *                 a connection has completed or rows are available).
 */
<<__Native, __HipHopSpecific>>
function mysql_async_wait_actionable(array<array<mixed>> $items,
                                     float $double): mixed;

/**
 * Returns the async operation status for a given mysql connection. For
 * non-async connections, this returns ASYNC_OP_INVALID. For an async
 * connection, this can be either ASYNC_OP_UNSET (no pending async operation),
 * ASYNC_OP_QUERY (async query pending), or ASYNC_OP_CONNECT (async connection
 * pending). Returns -1 if the supplied connection itself is invalid.
 *
 * @param resource $link_identifier - The MySQL connection
 *
 * @return int - Returns the async operation number for this mysql connection.
 */
<<__Native, __HipHopSpecific>>
function mysql_async_status(?resource $link_identifier = NULL): int;
