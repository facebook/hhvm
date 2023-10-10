<?hh

const int MYSQL_CLIENT_COMPRESS = 32;
const int MYSQL_CLIENT_IGNORE_SPACE = 256;
const int MYSQL_CLIENT_INTERACTIVE = 1024;
const int MYSQL_CLIENT_SSL = 2048;

/**
 * Get number of affected rows in previous MySQL operation
 *
 * @param resource $link_identifier -
 *
 * @return int - Returns the number of affected rows on success, and -1
 *   if the last query failed.   If the last query was a DELETE query with
 *   no WHERE clause, all of the records will have been deleted from the
 *   table but this function will return zero with MySQL versions prior to
 *   4.1.2.   When using UPDATE, MySQL will not update columns where the
 *   new value is the same as the old value. This creates the possibility
 *   that mysql_affected_rows() may not actually equal the number of rows
 *   matched, only the number of rows that were literally affected by the
 *   query.   The REPLACE statement first deletes the record with the same
 *   primary key and then inserts the new record. This function returns the
 *   number of deleted records plus the number of inserted records.   In
 *   the case of "INSERT ... ON DUPLICATE KEY UPDATE" queries, the return
 *   value will be 1 if an insert was performed, or 2 for an update of an
 *   existing row.
 */
<<__Native>>
function mysql_affected_rows(?resource $link_identifier = NULL): mixed;

/**
 * Returns the name of the character set
 *
 * @param resource $link_identifier -
 *
 * @return string - Returns the default character set name for the
 *   current connection.
 */
<<__Native>>
function mysql_client_encoding(?resource $link_identifier = NULL): mixed;

/**
 * Close MySQL connection
 *
 * @param resource $link_identifier -
 *
 * @return bool -
 */
<<__Native>>
function mysql_close(?resource $link_identifier = NULL): bool;

/**
 * Open a connection to a MySQL Server
 *
 * @param string $server - server   The MySQL server. It can also include
 *   a port number. e.g. "hostname:port" or a path to a local socket e.g.
 *   ":/path/to/socket" for the localhost.   If the PHP directive
 *   mysql.default_host is undefined (default), then the default value is
 *   'localhost:3306'. In , this parameter is ignored and value
 *   'localhost:3306' is always used.
 * @param string $username - username   The username. Default value is
 *   defined by mysql.default_user. In , this parameter is ignored and the
 *   name of the user that owns the server process is used.
 * @param string $password - password   The password. Default value is
 *   defined by mysql.default_password. In , this parameter is ignored and
 *   empty password is used.
 * @param bool $new_link - new_link   If a second call is made to
 *   mysql_connect() with the same arguments, no new link will be
 *   established, but instead, the link identifier of the already opened
 *   link will be returned. The new_link parameter modifies this behavior
 *   and makes mysql_connect() always open a new link, even if
 *   mysql_connect() was called before with the same parameters. In , this
 *   parameter is ignored.
 * @param int $client_flags - client_flags   The client_flags parameter
 *   can be a combination of the following constants: 128 (enable LOAD DATA
 *   LOCAL handling), MYSQL_CLIENT_SSL, MYSQL_CLIENT_COMPRESS,
 *   MYSQL_CLIENT_IGNORE_SPACE or MYSQL_CLIENT_INTERACTIVE. Read the
 *   section about  for further information. In , this parameter is
 *   ignored.
 * @param array $conn_attrs - connection attribute associative array
 *
 * @return resource - Returns a MySQL link identifier on success.
 */
<<__Native("NoFCallBuiltin")>>
function mysql_connect(string $server = "",
                       string $username = "",
                       string $password = "",
                       bool $new_link = false,
                       int $client_flags = 0,
                       int $connect_timeout_ms = -1,
                       int $query_timeout_ms = -1,
                       darray<string, string> $conn_attrs = darray[]): mixed;

<<__Native("NoFCallBuiltin")>>
function mysql_connect_with_db(string $server = "",
                               string $username = "",
                               string $password = "",
                               string $database = "",
                               bool $new_link = false,
                               int $client_flags = 0,
                               int $connect_timeout_ms = -1,
                               int $query_timeout_ms = -1,
                               darray<string, string> $conn_attrs = darray[]): mixed;

<<__Native("NoFCallBuiltin")>>
function mysql_connect_with_ssl(string $server,
                               string $username,
                               string $password,
                               string $database = "",
                               int $client_flags = 0,
                               int $connect_timeout_ms = -1,
                               int $query_timeout_ms = -1,
                               ?MySSLContextProvider $ssl_context = null,
                               darray<string, string> $conn_attrs = darray[]): mixed;

/**
 * Create a MySQL database
 *
 * @param string $database_name - database_name   The name of the
 *   database being created.
 * @param resource $link_identifier -
 *
 * @return bool -
 */
function mysql_create_db(string $database_name,
                         ?resource $link_identifier = NULL): bool {
  throw new Exception(sprintf(
    '%s is not supported: %s',
    __FUNCTION__,
    'Deprecated. Use mysql_query(CREATE DATABASE) instead.'
  ));
}

/**
 * Move internal result pointer
 *
 * @param resource $result -
 * @param int $row_number - row_number   The desired row number of the
 *   new result pointer.
 *
 * @return bool -
 */
<<__Native>>
function mysql_data_seek(resource $result,
                         int $row_number): bool;

/**
 * Retrieves database name from the call to
 *
 * @param resource $result - result   The result pointer from a call to
 *   mysql_list_dbs().
 * @param int $row - row   The index into the result set.
 * @param mixed $field - field   The field name.
 *
 * @return string - Returns the database name on success, and FALSE on
 *   failure. If FALSE is returned, use mysql_error() to determine the
 *   nature of the error.
 */
function mysql_db_name(resource $result,
                       mixed $row,
                       mixed $field = NULL): mixed {
  return mysql_result($result, $row, $field);
}

/**
 * Selects a database and executes a query on it
 *
 * @param string $database - database   The name of the database that
 *   will be selected.
 * @param string $query - query   The MySQL query.   Data inside the
 *   query should be properly escaped.
 * @param resource $link_identifier -
 *
 * @return resource - Returns a positive MySQL result resource to the
 *   query result, or FALSE on error. The function also returns TRUE/FALSE
 *   for INSERT/UPDATE/DELETE queries to indicate success/failure.
 */
function mysql_db_query(string $database,
                        string $query,
                        ?resource $link_identifier = NULL): resource {
  throw new Exception(sprintf(
    '%s is not supported: %s',
    __FUNCTION__,
    'Deprecated. Use mysql_query() instead.'
  ));
}

/**
 * Drop (delete) a MySQL database
 *
 * @param string $database_name - database_name   The name of the
 *   database that will be deleted.
 * @param resource $link_identifier -
 *
 * @return bool -
 */
function mysql_drop_db(string $database_name,
                       ?resource $link_identifier = NULL): bool {
  throw new Exception(sprintf(
    '%s is not supported: %s',
    __FUNCTION__,
    'Deprecated. Use mysql_query(DROP DATABASE) instead.'
  ));
}

/**
 * Returns the numerical value of the error message from previous MySQL
 * operation
 *
 * @param resource $link_identifier -
 *
 * @return int - Returns the error number from the last MySQL function,
 *   or 0 (zero) if no error occurred.
 */
<<__Native>>
function mysql_errno(?resource $link_identifier = NULL): mixed;

/**
 * Returns the text of the error message from previous MySQL operation
 *
 * @param resource $link_identifier -
 *
 * @return string - Returns the error text from the last MySQL function,
 *   or '' (empty string) if no error occurred.
 */
<<__Native>>
function mysql_error(?resource $link_identifier = NULL): mixed;

/**
 * Escapes a string for use in a mysql_query
 *
 * @param string $unescaped_string - unescaped_string   The string that
 *   is to be escaped.
 *
 * @return string - Returns the escaped string.
 */
<<__Native>>
function mysql_escape_string(string $unescaped_string)[]: string;

/**
 * Fetch a result row as an associative array, a numeric array, or both
 *
 * @param resource $result -
 * @param int $result_type - result_type   The type of array that is to
 *   be fetched. It's a constant and can take the following values:
 *   MYSQL_ASSOC, MYSQL_NUM, and MYSQL_BOTH.
 *
 * @return array - Returns an array of strings that corresponds to the
 *   fetched row, or FALSE if there are no more rows. The type of returned
 *   array depends on how result_type is defined. By using MYSQL_BOTH
 *   (default), you'll get an array with both associative and number
 *   indices. Using MYSQL_ASSOC, you only get associative indices (as
 *   mysql_fetch_assoc() works), using MYSQL_NUM, you only get number
 *   indices (as mysql_fetch_row() works).   If two or more columns of the
 *   result have the same field names, the last column will take
 *   precedence. To access the other column(s) of the same name, you must
 *   use the numeric index of the column or make an alias for the column.
 *   For aliased columns, you cannot access the contents with the original
 *   column name.
 */
<<__Native>>
function mysql_fetch_array(resource $result,
                           int $result_type = MYSQL_BOTH): mixed;

/**
 * Fetch a result row as an associative array
 *
 * @param resource $result -
 *
 * @return array - Returns an associative array of strings that
 *   corresponds to the fetched row, or FALSE if there are no more rows.
 *   If two or more columns of the result have the same field names, the
 *   last column will take precedence. To access the other column(s) of the
 *   same name, you either need to access the result with numeric indices
 *   by using mysql_fetch_row() or add alias names. See the example at the
 *   mysql_fetch_array() description about aliases.
 */
function mysql_fetch_assoc(resource $result): mixed {
  return mysql_fetch_array($result, MYSQL_ASSOC);
}

/**
 * Get column information from a result and return as an object
 *
 * @param resource $result -
 * @param int $field_offset - field_offset   The numerical field offset.
 *   If the field offset is not specified, the next field that was not yet
 *   retrieved by this function is retrieved. The field_offset starts at 0.
 *
 * @return object - Returns an object containing field information. The
 *   properties of the object are:      name - column name     table - name
 *   of the table the column belongs to, which is the alias name if one is
 *   defined     max_length - maximum length of the column     not_null - 1
 *   if the column cannot be NULL     primary_key - 1 if the column is a
 *   primary key     unique_key - 1 if the column is a unique key
 *   multiple_key - 1 if the column is a non-unique key     numeric - 1 if
 *   the column is numeric     blob - 1 if the column is a BLOB     type -
 *   the type of the column     unsigned - 1 if the column is unsigned
 *   zerofill - 1 if the column is zero-filled
 */
<<__Native>>
function mysql_fetch_field(resource $result,
                           int $field_offset = -1): mixed;

/**
 * Get the length of each output in a result
 *
 * @param resource $result -
 *
 * @return array - An array of lengths on success.
 */
<<__Native>>
function mysql_fetch_lengths(resource $result): mixed;

/**
 * Fetch a result row as an object
 *
 * @param resource $result -
 * @param string $class_name - class_name   The name of the class to
 *   instantiate, set the properties of and return. If not specified, a
 *   stdClass object is returned.
 * @param array $params - params   An optional array of parameters to
 *   pass to the constructor for class_name objects.
 *
 * @return object - Returns an object with string properties that
 *   correspond to the fetched row, or FALSE if there are no more rows.
 */
<<__Native>>
function mysql_fetch_object(mixed $result,
                            string $class_name = 'stdClass',
                            ?varray $params = null): mixed;

/**
 * Used with mysql_multi_query() to return a mysql result for the current
 * iterated query.
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
 * @return mixed - Returns a resource or a boolean
 */
<<__Native>>
function mysql_fetch_result(?resource $link_identifier = NULL): mixed;

/**
 * Get a result row as an enumerated array
 *
 * @param resource $result -
 *
 * @return array - Returns an numerical array of strings that corresponds
 *   to the fetched row, or FALSE if there are no more rows.
 *   mysql_fetch_row() fetches one row of data from the result associated
 *   with the specified result identifier. The row is returned as an array.
 *   Each result column is stored in an array offset, starting at offset 0.
 */
function mysql_fetch_row(resource $result): mixed {
  return mysql_fetch_array($result, MYSQL_NUM);
}

/**
 * Get the flags associated with the specified field in a result
 *
 * @param resource $result -
 * @param int $field_offset -
 *
 * @return string - Returns a string of flags associated with the result.
 *     The following flags are reported, if your version of MySQL is
 *   current enough to support them: "not_null", "primary_key",
 *   "unique_key", "multiple_key", "blob", "unsigned", "zerofill",
 *   "binary", "enum", "auto_increment" and "timestamp".
 */
<<__Native>>
function mysql_field_flags(resource $result,
                           int $field_offset): mixed;

/**
 * Returns the length of the specified field
 *
 * @param resource $result -
 * @param int $field_offset -
 *
 * @return int - The length of the specified field index on success.
 */
<<__Native>>
function mysql_field_len(resource $result,
                         int $field_offset): mixed;

/**
 * Get the name of the specified field in a result
 *
 * @param resource $result -
 * @param int $field_offset -
 *
 * @return string - The name of the specified field index on success.
 */
<<__Native>>
function mysql_field_name(resource $result,
                          int $field_offset): mixed;

/**
 * Set result pointer to a specified field offset
 *
 * @param resource $result -
 * @param int $field_offset -
 *
 * @return bool -
 */
<<__Native>>
function mysql_field_seek(resource $result,
                          int $field_offset): bool;

/**
 * Get name of the table the specified field is in
 *
 * @param resource $result -
 * @param int $field_offset -
 *
 * @return string - The name of the table on success.
 */
<<__Native>>
function mysql_field_table(resource $result,
                           int $field_offset): mixed;

/**
 * Get the type of the specified field in a result
 *
 * @param resource $result -
 * @param int $field_offset -
 *
 * @return string - The returned field type will be one of "int", "real",
 *   "string", "blob", and others as detailed in the MySQL documentation.
 */
<<__Native>>
function mysql_field_type(resource $result,
                          int $field_offset): mixed;

/**
 * Free result memory
 *
 * @param resource $result -
 *
 * @return bool - If a non-resource is used for the result, an error of
 *   level E_WARNING will be emitted. It's worth noting that mysql_query()
 *   only returns a resource for SELECT, SHOW, EXPLAIN, and DESCRIBE
 *   queries.
 */
<<__Native>>
function mysql_free_result(resource $result): bool;

/**
 * Get MySQL client info
 *
 * @return string - The MySQL client version.
 */
<<__Native>>
function mysql_get_client_info(): string;

/**
 * Get MySQL host info
 *
 * @param resource $link_identifier -
 *
 * @return string - Returns a string describing the type of MySQL
 *   connection in use for the connection.
 */
<<__Native>>
function mysql_get_host_info(?resource $link_identifier = NULL): mixed;

/**
 * Get MySQL protocol info
 *
 * @param resource $link_identifier -
 *
 * @return int - Returns the MySQL protocol on success.
 */
<<__Native>>
function mysql_get_proto_info(?resource $link_identifier = NULL): mixed;

/**
 * Get MySQL server info
 *
 * @param resource $link_identifier -
 *
 * @return string - Returns the MySQL server version on success.
 */
<<__Native>>
function mysql_get_server_info(?resource $link_identifier = NULL): mixed;

/**
 * Get information about the most recent query
 *
 * @param resource $link_identifier -
 *
 * @return string - Returns information about the statement on success,
 *   or FALSE on failure. See the example below for which statements
 *   provide information, and what the returned value may look like.
 *   Statements that are not listed will return FALSE.
 */
<<__Native>>
function mysql_info(?resource $link_identifier = NULL): mixed;

/**
 * Get the ID generated in the last query
 *
 * @param resource $link_identifier -
 *
 * @return int - The ID generated for an AUTO_INCREMENT column by the
 *   previous query on success, 0 if the previous query does not generate
 *   an AUTO_INCREMENT value, or FALSE if no MySQL connection was
 *   established.
 */
<<__Native>>
function mysql_insert_id(?resource $link_identifier = NULL): mixed;

/**
 * List databases available on a MySQL server
 *
 * @param resource $link_identifier -
 *
 * @return resource - Returns a result pointer resource on success, or
 *   FALSE on failure. Use the mysql_tablename() function to traverse this
 *   result pointer, or any function for result tables, such as
 *   mysql_fetch_array().
 */
<<__Native>>
function mysql_list_dbs(?resource $link_identifier = NULL): mixed;

/**
 * List MySQL table fields
 *
 * @param string $database_name - database_name   The name of the
 *   database that's being queried.
 * @param string $table_name - table_name   The name of the table that's
 *   being queried.
 * @param resource $link_identifier -
 *
 * @return resource - A result pointer resource on success, or FALSE on
 *   failure.   The returned result can be used with mysql_field_flags(),
 *   mysql_field_len(), mysql_field_name() mysql_field_type().
 */
function mysql_list_fields(string $database_name,
                           string $table_name,
                           ?resource $link_identifier = NULL): resource {
  throw new Exception(sprintf(
    '%s is not supported: %s',
    __FUNCTION__,
    'Deprecated. Use mysql_query(SHOW COLUMNS FROM table [LIKE \'name\']) ' .
      'instead.'
  ));
}

/**
 * List MySQL processes
 *
 * @param resource $link_identifier -
 *
 * @return resource - A result pointer resource on success.
 */
<<__Native>>
function mysql_list_processes(?resource $link_identifier = NULL): mixed;

/**
 * List tables in a MySQL database
 *
 * @param string $database - database   The name of the database
 * @param resource $link_identifier -
 *
 * @return resource - A result pointer resource on success.   Use the
 *   mysql_tablename() function to traverse this result pointer, or any
 *   function for result tables, such as mysql_fetch_array().
 */
<<__Native>>
function mysql_list_tables(string $database,
                           ?resource $link_identifier = NULL): mixed;

/**
 * Used with mysql_multi_query() to check if there are more result sets to be
 * returned.
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
 * @return bool - True if there is at least one more item in the result set.
 */
<<__Native>>
function mysql_more_results(?resource $link_identifier = NULL): bool;

/**
 * mysql_multi_query() executes one or more queries separated by a ; to the
 * currently active database on the server that's associated with the specified
 * link_identifier.
 *
 * @param string $query             - An SQL query
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
 * @return mixed - This is a fb specific query so behaviour is a little random
 *                 at the moment.
 */
<<__Native>>
function mysql_multi_query(string $query,
                           ?resource $link_identifier = NULL): mixed;

/**
 * Used with mysql_multi_query() to move the result set on one.
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
 * @return int - 0 - Query succeeded, more results coming. -1 - Query succeeded,
 *               no more results coming. >0 - query failed, value is error code.
 */
<<__Native>>
function mysql_next_result(?resource $link_identifier = NULL): int;

/**
 * Get number of fields in result
 *
 * @param resource $result -
 *
 * @return int - Returns the number of fields in the result set resource
 *   on success.
 */
<<__Native>>
function mysql_num_fields(resource $result): mixed;

/**
 * Get number of rows in result
 *
 * @param resource $result -
 *
 * @return int - The number of rows in a result set on success.
 */
<<__Native>>
function mysql_num_rows(resource $result): mixed;

/**
 * Open a persistent connection to a MySQL server
 *
 * @param string $server - server   The MySQL server. It can also include
 *   a port number. e.g. "hostname:port" or a path to a local socket e.g.
 *   ":/path/to/socket" for the localhost.   If the PHP directive
 *   mysql.default_host is undefined (default), then the default value is
 *   'localhost:3306'
 * @param string $username - username   The username. Default value is
 *   the name of the user that owns the server process.
 * @param string $password - password   The password. Default value is an
 *   empty password.
 * @param int $client_flags - client_flags   The client_flags parameter
 *   can be a combination of the following constants: 128 (enable LOAD DATA
 *   LOCAL handling), MYSQL_CLIENT_SSL, MYSQL_CLIENT_COMPRESS,
 *   MYSQL_CLIENT_IGNORE_SPACE or MYSQL_CLIENT_INTERACTIVE.
 *
 * @return resource - Returns a MySQL persistent link identifier on
 *   success, or FALSE on failure.
 */
<<__Native>>
function mysql_pconnect(string $server = '',
                        string $username = '',
                        string $password = '',
                        int $client_flags = 0,
                        int $connect_timeout_ms = -1,
                        int $query_timeout_ms = -1,
                        darray<string, string> $conn_attrs = darray[]): mixed;

<<__Native>>
function mysql_pconnect_with_db(string $server = '',
                                string $username = '',
                                string $password = '',
                                string $database = '',
                                int $client_flags = 0,
                                int $connect_timeout_ms = -1,
                                int $query_timeout_ms = -1,
                                darray<string, string> $conn_attrs = darray[]): mixed;

/**
 * Ping a server connection or reconnect if there is no connection
 *
 * @param resource $link_identifier -
 *
 * @return bool - Returns TRUE if the connection to the server MySQL
 *   server is working, otherwise FALSE.
 */
<<__Native>>
function mysql_ping(?resource $link_identifier = NULL): ?bool;

/**
 * Send a MySQL query
 *
 * @param string $query - query   An SQL query   The query string should
 *   not end with a semicolon. Data inside the query should be properly
 *   escaped.
 * @param resource $link_identifier -
 *
 * @return resource - For SELECT, SHOW, DESCRIBE, EXPLAIN and other
 *   statements returning resultset, mysql_query() returns a resource on
 *   success, or FALSE on error.   For other type of SQL statements,
 *   INSERT, UPDATE, DELETE, DROP, etc, mysql_query() returns TRUE on
 *   success or FALSE on error.   The returned result resource should be
 *   passed to mysql_fetch_array(), and other functions for dealing with
 *   result tables, to access the returned data.   Use mysql_num_rows() to
 *   find out how many rows were returned for a SELECT statement or
 *   mysql_affected_rows() to find out how many rows were affected by a
 *   DELETE, INSERT, REPLACE, or UPDATE statement.   mysql_query() will
 *   also fail and return FALSE if the user does not have permission to
 *   access the table(s) referenced by the query.
 */
<<__Native("NoFCallBuiltin")>>
function mysql_query(string $query,
                     ?resource $link_identifier = NULL): mixed;

/**
 * Escapes special characters in a string for use in an SQL statement
 *
 * @param string $unescaped_string - unescaped_string   The string that
 *   is to be escaped.
 * @param resource $link_identifier -
 *
 * @return string - Returns the escaped string, or FALSE on error.
 */
<<__Native>>
function mysql_real_escape_string(string $unescaped_string,
                                  ?resource $link_identifier = NULL): mixed;

/**
 * Get result data
 *
 * @param resource $result -
 * @param int $row - row   The row number from the result that's being
 *   retrieved. Row numbers start at 0.
 * @param mixed $field - field   The name or offset of the field being
 *   retrieved.   It can be the field's offset, the field's name, or the
 *   field's table dot field name (tablename.fieldname). If the column name
 *   has been aliased ('select foo as bar from...'), use the alias instead
 *   of the column name. If undefined, the first field is retrieved.
 *
 * @return string - The contents of one cell from a MySQL result set on
 *   success, or FALSE on failure.
 */
<<__Native>>
function mysql_result(resource $result,
                      int $row,
                      mixed $field = 0): mixed;

/**
 * Select a MySQL database
 *
 * @param string $database_name - database_name   The name of the
 *   database that is to be selected.
 * @param resource $link_identifier -
 *
 * @return bool -
 */
<<__Native>>
function mysql_select_db(string $database_name,
                         ?resource $link_identifier = NULL): bool;

/**
 * Sets the client character set
 *
 * @param string $charset - charset   A valid character set name.
 * @param resource $link_identifier -
 *
 * @return bool -
 */
<<__Native>>
function mysql_set_charset(string $charset,
                           ?resource $link_identifier = NULL): ?bool;

/**
 * Sets query timeout for a connection
 *
 * @param int $query_timeout_ms     - How many milli-seconds to wait for an SQL
 *                                    query
 * @param resource $link_identifier - Which connection to set to. If absent,
 *                                    default or current connection will be
 *                                    applied to.
 *
 * @return bool
 */
<<__Native>>
function mysql_set_timeout(int $query_timeout_ms,
                           ?resource $link_identifier = NULL): bool;

/**
 * Get current system status
 *
 * @param resource $link_identifier -
 *
 * @return string - Returns a string with the status for uptime, threads,
 *   queries, open tables, flush tables and queries per second. For a
 *   complete list of other status variables, you have to use the SHOW
 *   STATUS SQL command. If link_identifier is invalid, NULL is returned.
 */
<<__Native>>
function mysql_stat(?resource $link_identifier = NULL): mixed;

/**
 * Get table name of field
 *
 * @param resource $result - result   A result pointer resource that's
 *   returned from mysql_list_tables().
 * @param int $i - i   The integer index (row/table number)
 *
 * @return string - The name of the table on success.   Use the
 *   mysql_tablename() function to traverse this result pointer, or any
 *   function for result tables, such as mysql_fetch_array().
 */
function mysql_tablename(resource $result,
                         int $i): mixed {
  return mysql_result($result, $i);
}

/**
 * Return the current thread ID
 *
 * @param resource $link_identifier -
 *
 * @return int - The thread ID on success.
 */
<<__Native>>
function mysql_thread_id(?resource $link_identifier = NULL): mixed;

/**
 * Send an SQL query to MySQL without fetching and buffering the result rows.
 *
 * @param string $query - query   The SQL query to execute.   Data inside
 *   the query should be properly escaped.
 * @param resource $link_identifier -
 *
 * @return resource - For SELECT, SHOW, DESCRIBE or EXPLAIN statements,
 *   mysql_unbuffered_query() returns a resource on success, or FALSE on
 *   error.   For other type of SQL statements, UPDATE, DELETE, DROP, etc,
 *   mysql_unbuffered_query() returns TRUE on success or FALSE on error.
 */
<<__Native>>
function mysql_unbuffered_query(string $query,
                                ?resource $link_identifier = NULL): mixed;

/**
 * Returns the number of errors generated during execution of the previous SQL
 * statement. To retrieve warning messages you can use the SQL command SHOW
 * WARNINGS [limit row_count].
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
 * @return int - Returns the number of warnings from the last MySQL function, or
 *               0 (zero) if no warnings occurred.
 */
<<__Native>>
function mysql_warning_count(?resource $link_identifier = NULL): mixed;
