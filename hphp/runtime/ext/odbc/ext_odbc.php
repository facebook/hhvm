<?hh

/**
 * odbc_autocommit() - http://php.net/function.odbc_autocommit
 *
 * @param resource $connection_id  - The ODBC connection identifier,
 *                                   see odbc_connect() for details.
 * @param bool $OnOff              - If OnOff is TRUE, auto-commit is enabled,
 *                                   if it is FALSE auto-commit is disabled.
 *
 * @return bool       - Without the OnOff parameter, this function returns
 *                      auto-commit status for connection_id. Non-zero is
 *                      returned if auto-commit is on, 0 if it is off, or
 *                      FALSE if an error occurs. If OnOff is set, this
 *                      function returns TRUE on success and FALSE on failure.
 */
<<__Native>> /* private */
function odbc_get_autocommit(resource $connection_id): bool;

<<__Native>> /* private */
function odbc_set_autocommit(resource $connection_id, bool $OnOff): bool;

function odbc_autocommit(resource $connection_id, ?bool $OnOff = false): bool {
  if (func_num_args() == 1) {
    return odbc_get_autocommit($connection_id);
  } else {
    return odbc_set_autocommit($connection_id, $OnOff);
  }
}


/**
 * odbc_commit() - http://php.net/function.odbc_commit
 *
 * @param resource $connection_id  - The ODBC connection identifier,
 *                                   see odbc_connect() for details.
 *
 * @return bool           - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function odbc_commit(resource $connection_id): bool;


/**
 * odbc_connect() - http://php.net/function.odbc_connect
 *
 * @param string $dsn      - The database source name for the connection.
 *                           Alternatively, a DSN-less connection string can
 *                           be used.
 * @param string $user     - The username.
 * @param string $password - The password.
 * @param int $cursor_type - This sets the type of cursor to be used for this
 *                           connection. This parameter is not normally needed,
 *                           but can be useful for working around problems with
 *                           some ODBC drivers. (not implemented yet). The
 *                           following constants are defined for cursortype:
 *                            SQL_CUR_USE_IF_NEEDED
 *                            SQL_CUR_USE_ODBC
 *                            SQL_CUR_USE_DRIVER
 *
 * @return mixed    - Returns an ODBC connection or (FALSE) on error.
 */
<<__Native>>
function odbc_connect(string $dsn, string $user, string $password,
                        ?int $cursor_type=0): mixed;

/**
 * odbc_close() - http://php.net/function.odbc_close
 *
 * @param resource $connection_id  - The ODBC connection identifier,
 *                                   see odbc_connect() for details.
 *
 * @return void    - No value is returned.
 */
<<__Native>>
function odbc_close(resource $connection_id): void;


/**
 * odbc_error() - http://php.net/function.odbc_error
 *
 * @param resource $connection_id  - The ODBC connection identifier,
 *                                   see odbc_connect() for details.
 *
 * @return string     - If connection_id is specified, the last state of that
 *                      connection is returned, else the last state of any
 *                      connection is returned. This function returns meaningful
 *                      value only if last odbc query failed (i.e. odbc_exec()
 *                      returned FALSE).
 */
<<__Native>>
function odbc_error(?resource $connection_id = NULL): string;


/**
 * odbc_errormsg() - http://php.net/function.odbc_errormsg
 *
 * @param resource $connection_id  - The ODBC connection identifier,
 *                                   see odbc_connect() for details.
 *
 * @return string     - If connection_id is specified, the last state of that
 *                      connection is returned, else the last state of any
 *                      connection is returned. This function returns meaningful
 *                      value only if last odbc query failed (i.e. odbc_exec()
 *                      returned FALSE).
 */
<<__Native>>
function odbc_errormsg(?resource $connection_id = NULL): string;


/**
 * odbc_exec() - http://php.net/function.odbc_exec
 *
 * @param resource $connection_id  - The ODBC connection identifier,
 *                                   see odbc_connect() for details.
 * @param string $query_string     - The SQL statement.
 * @param int $flags               - This parameter is currently not used.
 *
 * @return mixed                   - Returns an ODBC result identifier if the
 *                                   SQL command was executed successfully, or
 *                                   FALSE on error.
 */
<<__Native>>
function odbc_exec(resource $connection_id, string $query_string,
                      ?int $flags=0): mixed;


/**
 * odbc_execute() - http://php.net/function.odbc_execute
 *
 * @param resource $connection_id  - The ODBC connection identifier,
 *                                   see odbc_connect() for details.
 * @param string $parameters       - Parameters in $parameters will be
 *                                   substituted for placeholders in the
 *                                   prepared statement in order.
 *
 * @return bool                - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function odbc_execute(resource $result, ?array $parameters_array): bool;


/**
 * odbc_fetch_array() - http://php.net/function.odbc_fetch_array
 *
 * @param resource $result  - The result resource from odbc_exec().
 * @param int $rownumber    - Optionally choose which row number to
 *                          retrieve (not implemented yet).
 *
 * @return mixed            - Returns an array that corresponds to the fetched
 *                            row, or FALSE if there are no more rows.
 */
<<__Native>>
function odbc_fetch_array(resource $result , ?int $rownumber=0): mixed;


/**
 * odbc_num_rows() - http://php.net/function.odbc_num_rows
 *
 * @param resource $result  - The result resource from odbc_exec().
 *
 * @return mixed            - Returns the number of rows in an ODBC result.
 *                            This function will return -1 on error.
 */
<<__Native>>
function odbc_num_rows(resource $result): int;


/**
 * odbc_prepare() - http://php.net/function.odbc_prepare
 *
 * @param resource $connection_id  - The ODBC connection identifier,
 *                                   see odbc_connect() for details.
 * @param string $query_string     - The SQL statement.
 * @param int $flags               - This parameter is currently not used.
 *
 * @return mixed                   - Returns an ODBC result identifier if the
 *                                   SQL command was executed successfully, or
 *                                   FALSE on error.
 */
<<__Native>>
function odbc_prepare(resource $connection_id, string $query_string): mixed;


/**
 * odbc_rollback() - http://php.net/function.odbc_rollback
 *
 * @param resource $connection_id  - The ODBC connection identifier,
 *                                   see odbc_connect() for details.
 *
 * @return bool           - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function odbc_rollback(resource $connection_id): bool;



/* Handling of binary column data */
function odbc_binmode() {
  throw new Exception('Not implemented yet');
}

/* Close all ODBC connections */
function odbc_close_all() {
  throw new Exception('Not implemented yet');
}

/* Lists columns and associated privileges for the given table */
function odbc_columnprivileges() {
  throw new Exception('Not implemented yet');
}

/* Lists the column names in specified tables */
function odbc_columns() {
  throw new Exception('Not implemented yet');
}

/* Get cursorname */
function odbc_cursor() {
  throw new Exception('Not implemented yet');
}

/* Returns information about a current connection */
function odbc_data_source() {
  throw new Exception('Not implemented yet');
}

/* Alias of odbc_exec */
function odbc_do() {
  throw new Exception('Not implemented yet');
}

/* Fetch one result row into array */
function odbc_fetch_into() {
  throw new Exception('Not implemented yet');
}

/* Fetch a result row as an object */
function odbc_fetch_object() {
  throw new Exception('Not implemented yet');
}

/* Fetch a row */
function odbc_fetch_row() {
  throw new Exception('Not implemented yet');
}

/* Get the length (precision) of a field */
function odbc_field_len() {
  throw new Exception('Not implemented yet');
}

/* Get the columnname */
function odbc_field_name() {
  throw new Exception('Not implemented yet');
}

/* Return column number */
function odbc_field_num() {
  throw new Exception('Not implemented yet');
}

/* Alias of odbc_field_len */
function odbc_field_precision() {
  throw new Exception('Not implemented yet');
}

/* Get the scale of a field */
function odbc_field_scale() {
  throw new Exception('Not implemented yet');
}

/* Datatype of a field */
function odbc_field_type() {
  throw new Exception('Not implemented yet');
}

/* Retrieves a list of foreign keys */
function odbc_foreignkeys() {
  throw new Exception('Not implemented yet');
}

/* Free resources associated with a result */
function odbc_free_result() {
  throw new Exception('Not implemented yet');
}

/* Retrieves information about data types supported by the data source */
function odbc_gettypeinfo() {
  throw new Exception('Not implemented yet');
}

/* Handling of LONG columns */
function odbc_longreadlen() {
  throw new Exception('Not implemented yet');
}

/* Checks if multiple results are available */
function odbc_next_result() {
  throw new Exception('Not implemented yet');
}

/* Number of columns in a result */
function odbc_num_fields() {
  throw new Exception('Not implemented yet');
}

/* Open a persistent database connection */
function odbc_pconnect() {
  throw new Exception('Not implemented yet');
}

/* Gets the primary keys for a table */
function odbc_primarykeys() {
  throw new Exception('Not implemented yet');
}

/* Retrieve information about parameters to procedures */
function odbc_procedurecolumns() {
  throw new Exception('Not implemented yet');
}

/* Get the list of procedures stored in a specific data source */
function odbc_procedures() {
  throw new Exception('Not implemented yet');
}

/* Print result as HTML table */
function odbc_result_all() {
  throw new Exception('Not implemented yet');
}

/* Get result data */
function odbc_result() {
  throw new Exception('Not implemented yet');
}

/* Adjust ODBC settings */
function odbc_setoption() {
  throw new Exception('Not implemented yet');
}

/* Retrieves special columns */
function odbc_specialcolumns() {
  throw new Exception('Not implemented yet');
}

/* Retrieve statistics about a table */
function odbc_statistics() {
  throw new Exception('Not implemented yet');
}

/* Lists tables and the privileges associated with each table */
function odbc_tableprivileges() {
  throw new Exception('Not implemented yet');
}

/* Get the list of table names stored in a specific data source */
function odbc_tables() {
  throw new Exception('Not implemented yet');
}
