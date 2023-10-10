<?hh

/**
 * A class that interfaces SQLite 3 databases.
 *
 */
<<__NativeData>>
class SQLite3 {

  /**
   * @param string $filename - Path to the SQLite database.
   * @param int $flags - Optional flags used to determine how to open the
   *   SQLite database. By default, open uses SQLITE3_OPEN_READWRITE |
   *   SQLITE3_OPEN_CREATE.  SQLITE3_OPEN_READONLY: Open the database for
   *   reading only.  SQLITE3_OPEN_READWRITE: Open the database for reading and
   *   writing.  SQLITE3_OPEN_CREATE: Create the database if it does not exist.
   * @param ?string $encryption_key - An optional encryption key used when
   *   encrypting and decrypting an SQLite database.
   *
   */
  <<__Native>>
  public function __construct(string $filename,
                       int $flags = SQLITE3_OPEN_READWRITE |
                         SQLITE3_OPEN_CREATE,
                       ?string $encryption_key = null): void;

  /**
   * Opens an SQLite 3 Database. If the build includes encryption, then it
   *   will attempt to use the key.
   *
   * @param string $filename - Path to the SQLite database.
   * @param int $flags - Optional flags used to determine how to open the
   *   SQLite database. By default, open uses SQLITE3_OPEN_READWRITE |
   *   SQLITE3_OPEN_CREATE.  SQLITE3_OPEN_READONLY: Open the database for
   *   reading only.  SQLITE3_OPEN_READWRITE: Open the database for reading and
   *   writing.  SQLITE3_OPEN_CREATE: Create the database if it does not exist.
   * @param ?string $encryption_key - An optional encryption key used when
   *   encrypting and decrypting an SQLite database.
   *
   */
  <<__Native>>
  public function open(string $filename,
                int $flags = SQLITE3_OPEN_READWRITE | SQLITE3_OPEN_CREATE,
                ?string $encryption_key = null): void;

  /**
   * Sets a busy handler that will sleep until the database is not locked or
   *   the timeout is reached.
   *
   * @param int $msecs - The milliseconds to sleep. Setting this value to a
   *   value less than or equal to zero, will turn off an already set timeout
   *   handler.
   *
   * @return bool - Returns TRUE on success, FALSE on failure.
   *
   */
  <<__Native>>
  public function busytimeout(int $msecs): bool;

  /**
   * Closes the database connection.
   *
   * @return bool - Returns TRUE on success, FALSE on failure.
   *
   */
  <<__Native>>
  public function close(): bool;

  /**
   * Executes a result-less query against a given database.
   *
   * @param string $sql - The SQL query to execute (typically an INSERT,
   *   UPDATE, or DELETE query).
   *
   * @return bool - Returns TRUE if the query succeeded, FALSE on failure.
   *
   */
  <<__Native>>
  public function exec(string $sql): bool;

  /**
   * Returns the SQLite3 library version as a string constant and as a number.
   *
   * @return array - Returns an associative array with the keys
   *   "versionString" and "versionNumber".
   *
   */
  <<__Native>>
  public static function version(): shape(
    'versionString' => string,
    'versionNumber' => int,
  );

  /**
   * Returns the row ID of the most recent INSERT into the database.
   *
   * @return int - Returns the row ID of the most recent INSERT into the
   *   database
   *
   */
  <<__Native>>
  public function lastinsertrowid(): int;

  /**
   * Returns the numeric result code of the most recent failed SQLite request.
   *
   * @return int - Returns an integer value representing the numeric result
   *   code of the most recent failed SQLite request.
   *
   */
  <<__Native>>
  public function lasterrorcode(): int;

  /**
   * Returns English text describing the most recent failed SQLite request.
   *
   * @return string - Returns an English string describing the most recent
   *   failed SQLite request.
   *
   */
  <<__Native>>
  public function lasterrormsg(): string;

  /**
   * Attempts to load an SQLite extension library.
   *
   * @param string $extension - The name of the library to load. The library
   *   must be located in the directory specified in the configure option
   *   sqlite3.extension_dir.
   *
   * @return bool - Returns TRUE if the extension is successfully loaded,
   *   FALSE on failure.
   *
   */
  <<__Native>>
  public function loadExtension(string $extension): bool;

  /**
   * Returns the number of database rows that were changed (or inserted or
   *   deleted) by the most recent SQL statement.
   *
   * @return int - Returns an integer value corresponding to the number of
   *   database rows changed (or inserted or deleted) by the most recent SQL
   *   statement.
   *
   */
  <<__Native>>
  public function changes(): int;

  /**
   * Returns a string that has been properly escaped for safe inclusion in an
   *   SQL statement.
   *
   * @param string $sql - The string to be escaped.
   *
   * @return string - Returns a properly escaped string that may be used
   *   safely in an SQL statement.
   *
   */
  <<__Native>>
  public static function escapestring(string $sql): string;

  /**
   * Prepares an SQL statement for execution and returns an SQLite3Stmt
   *   object.
   *
   * @param string $sql - The SQL query to prepare.
   *
   * @return mixed - Returns an SQLite3Stmt object on success or FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function prepare(string $sql): mixed;

  /**
   * Executes an SQL query, returning an SQLite3Result object if the query
   *   returns results.
   *
   * @param string $sql - The SQL query to execute.
   *
   * @return mixed - Returns an SQLite3Result object if the query returns
   *   results. Otherwise, returns TRUE if the query succeeded, FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function query(string $sql): mixed;

  /**
   * Executes a query and returns a single result.
   *
   * @param string $sql - The SQL query to execute.
   * @param bool $entire_row - By default, querySingle returns the value of
   *   the first column returned by the query. If entire_row is TRUE, then it
   *   returns an array of the entire first row.
   *
   * @return mixed - Returns the value of the first column of results or an
   *   array of the entire first row (if entire_row is TRUE), otherwise FALSE on
   *   failure.
   *
   */
  <<__Native>>
  public function querysingle(string $sql, bool $entire_row = false): mixed;

  /**
   * Registers a PHP function or user-defined function for use as an SQL
   *   scalar function for use within SQL statements.
   *
   * @param string $name - Name of the SQL function to be created or
   *   redefined.
   * @param mixed $callback - The name of a PHP function or user-defined
   *   function to apply as a callback, defining the behavior of the SQL
   *   function.
   * @param int $argcount - The number of arguments that the SQL function
   *   takes. If this parameter is negative, then the SQL function may take any
   *   number of arguments.
   *
   * @return bool - Returns TRUE upon successful creation of the function,
   *   FALSE on failure.
   *
   */
  <<__Native>>
  public function createfunction(string $name,
                          mixed $callback,
                          int $argcount = -1): bool;

  /**
   * Registers a PHP function or user-defined function for use as an SQL
   *   aggregate function for use within SQL statements.
   *
   * @param string $name - Name of the SQL aggregate to be created or
   *   redefined.
   * @param mixed $step - The name of a PHP function or user-defined function
   *   to apply as a callback for every item in the aggregate.
   * @param mixed $final - The name of a PHP function or user-defined function
   *   to apply as a callback at the end of the aggregate data.
   * @param int $argcount - The number of arguments that the SQL aggregate
   *   takes. If this parameter is negative, then the SQL aggregate may take any
   *   number of arguments.
   *
   * @return bool - Returns TRUE upon successful creation of the aggregate,
   *   FALSE on failure.
   *
   */
  <<__Native>>
  public function createaggregate(string $name,
                           mixed $step,
                           mixed $final,
                           int $argcount = -1): bool;

  <<__Native>>
  public function openblob(string $table,
                    string $column,
                    int $rowid,
                    ?string $dbname = null): bool;
}

/**
 * A class that handles prepared statements for the SQLite 3 extension.
 *
 */
<<__NativeData>>
class SQLite3Stmt {

  <<__Native>>
  public function __construct(SQLite3 $dbobject, string $statement): void;

  /**
   * Returns the number of parameters within the prepared statement.
   *
   * @return int - Returns the number of parameters within the prepared
   *   statement.
   *
   */
  <<__Native>>
  public function paramcount(): int;

  /**
   * Closes the prepared statement.
   *
   * @return bool - Returns TRUE
   *
   */
  <<__Native>>
  public function close(): bool;

  /**
   * Resets the prepared statement to its state prior to execution. All
   *   bindings remain intact after reset.
   *
   * @return bool - Returns TRUE if the statement is successfully reset, FALSE
   *   on failure.
   *
   */
  <<__Native>>
  public function reset(): bool;

  /**
   * Clears all current bound parameters.
   *
   * @return bool - Returns TRUE on successful clearing of bound parameters,
   *   FALSE on failure.
   *
   */
  <<__Native>>
  public function clear(): bool;

  /**
   * Binds the value of a parameter to a statement variable.
   *
   * @param mixed $name - An string identifying the statement variable to
   *   which the value should be bound.
   * @param mixed $parameter - The value to bind to a statement variable.
   * @param int $type - The data type of the value to bind.  SQLITE3_INTEGER:
   *   The value is a signed integer, stored in 1, 2, 3, 4, 6, or 8 bytes
   *   depending on the magnitude of the value.  SQLITE3_FLOAT: The value is a
   *   floating point value, stored as an 8-byte IEEE floating point number.
   *   SQLITE3_TEXT: The value is a text string, stored using the database
   *   encoding (UTF-8, UTF-16BE or UTF-16-LE).  SQLITE3_BLOB: The value is a
   *   blob of data, stored exactly as it was input.  SQLITE3_NULL: The value is
   *   a NULL value.
   *
   * @return bool - Returns TRUE if the value is bound to the statement
   *   variable, FALSE on failure.
   *
   */
  <<__Native>>
  public function bindvalue(mixed $name,
                     mixed $parameter,
                     int $type = SQLITE3_TEXT): bool;

  /**
   * Executes a prepared statement and returns a result set object.
   *
   * @return mixed - Returns an SQLite3Result object on successful execution
   *   of the prepared statement, FALSE on failure.
   *
   */
  <<__Native>>
  public function execute(): mixed;
}

/**
 * A class that handles result sets for the SQLite 3 extension.
 *
 */
<<__NativeData>>
class SQLite3Result {

  public function __construct(): void {}

  /**
   * Returns the number of columns in the result set.
   *
   * @return int - Returns the number of columns in the result set.
   *
   */
  <<__Native>>
  public function numcolumns(): int;

  /**
   * Returns the name of the column specified by the column_number.
   *
   * @param int $column - The numeric zero-based index of the column.
   *
   * @return string - Returns the string name of the column identified by
   *   column_number.
   *
   */
  <<__Native>>
  public function columnname(int $column): string;

  /**
   * Returns the type of the column identified by column_number.
   *
   * @param int $column - The numeric zero-based index of the column.
   *
   * @return int - Returns the data type index of the column identified by
   *   column_number (one of SQLITE3_INTEGER, SQLITE3_FLOAT, SQLITE3_TEXT,
   *   SQLITE3_BLOB, or SQLITE3_NULL).
   *
   */
  <<__Native>>
  public function columntype(int $column): int;

  /**
   * Fetches a result row as an associative or numerically indexed array or
   *   both. By default, fetches as both.
   *
   * @param int $mode - Controls how the next row will be returned to the
   *   caller. This value must be one of either SQLITE3_ASSOC, SQLITE3_NUM, or
   *   SQLITE3_BOTH.  SQLITE3_ASSOC: returns an array indexed by column name as
   *   returned in the corresponding result set  SQLITE3_NUM: returns an array
   *   indexed by column number as returned in the corresponding result set,
   *   starting at column 0  SQLITE3_BOTH: returns an array indexed by both
   *   column name and number as returned in the corresponding result set,
   *   starting at column 0
   *
   * @return mixed - Returns a result row as an associatively or numerically
   *   indexed array or both.
   *
   */
  <<__Native>>
  public function fetcharray(int $mode = SQLITE3_BOTH): mixed;

  /**
   * @return bool
   *
   */
  <<__Native>>
  public function reset(): bool;

  /**
   * Closes the result set.
   *
   * @return bool - Returns TRUE.
   *
   */
  <<__Native>>
  public function finalize(): bool;
}
