<?hh

/**
 * Represents a connection between PHP and a MySQL database.
 */
class mysqli {

  public static int $__connection_errno = 0;
  public static ?string $__connection_error = null;

  private ?resource $__connection = null;

  <<__Native>>
  private function hh_get_connection(int $state = 0): ?resource;

  /**
   * Open a new connection to the MySQL server
   *
   * @param string $host - Can be either a host name or an IP address.
   *   Passing the NULL value or the string "localhost" to this parameter,
   *   the local host is assumed. When possible, pipes will be used instead
   *   of the TCP/IP protocol.   Prepending host by p: opens a persistent
   *   connection. mysqli_change_user() is automatically called on
   *   connections opened from the connection pool.
   * @param string $username - The MySQL user name.
   * @param string $passwd - If not provided or NULL, the MySQL server will
   *   attempt to authenticate the user against those user records which have
   *   no password only. This allows one username to be used with different
   *   permissions (depending on if a password as provided or not).
   * @param string $dbname - If provided will specify the default database
   *   to be used when performing queries.
   * @param mixed $port - Specifies the port number to attempt to connect to
   *   the MySQL server.
   * @param string $socket - Specifies the socket or named pipe that should
   *   be used.    Specifying the socket parameter will not explicitly
   *   determine the type of connection to be used when connecting to the
   *   MySQL server. How the connection is made to the MySQL database is
   *   determined by the host parameter.
   */
  public function __construct(?string $host = null,
                              ?string $username = null,
                              ?string $passwd = null,
                              ?string $dbname = null,
                              mixed $port = null,
                              ?string $socket = null): void {
    $this->hh_init();

    if (func_num_args() == 0) {
      return;
    }

    // If any of the necessary mysqli properties come in as null, then we can
    // use our default ini options.
    $host = $this->get_ini_default_if_null($host, "host");
    $username = $this->get_ini_default_if_null($username, "user");
    $passwd = $this->get_ini_default_if_null($passwd, "pw");
    $port = $this->get_ini_default_if_null($port, "port");
    $socket = $this->get_ini_default_if_null($socket, "socket");

    // Connect
    $this->real_connect($host, $username, $passwd, $dbname, $port, $socket);
  }

  private function get_ini_default_if_null(mixed $connect_option,
                                           string $name) {
    if ($connect_option === null) {
      $connect_option = ini_get("mysqli.default_" . $name);
    }
    return $connect_option;
  }

  public function __clone(): void {
    throw new Exception(
      'Trying to clone an uncloneable object of class mysqli'
    );
  }

  /**
   * Alias for __construct
   */
  public function connect(?string $host = null,
                          ?string $username = null,
                          ?string $passwd = null,
                          ?string $dbname = null,
                          mixed $port = null,
                          ?string $socket = null): void {
    $this->hh_init();

    if (func_num_args() == 0) {
      return;
    }

    // Connect
    $this->real_connect($host, $username, $passwd, $dbname, $port, $socket);
  }

  <<__Native>>
  private function hh_init(): void;

  /**
   * Turns on or off auto-committing database modifications
   *
   * @param bool $mode - Whether to turn on auto-commit or not.
   *
   * @return bool -
   */
  <<__Native>>
  public function autocommit(bool $mode): mixed;

  /**
   * Starts a transaction
   *
   * @param ?int $flags -
   * @param ?string $name -
   *
   * @return bool -
   */
  public function begin_transaction(?int $flags = null,
                                    ?string $name = null): bool {
    $query = 'START TRANSACTION';
    if ($name) {
      $query .= '/*'. $name .'*/';
    }

    if ($flags) {
      $option_strings = Map {
        MYSQLI_TRANS_START_WITH_CONSISTENT_SNAPSHOT =>
          'WITH CONSISTENT SNAPSHOT',
        MYSQLI_TRANS_START_READ_WRITE => 'READ WRITE',
        MYSQLI_TRANS_START_READ_ONLY => 'READ ONLY',
      };

      $options = array();
      foreach ($option_strings as $bit => $str) {
        if ($flags & $bit) {
          $options[] = $str;
        }
      }

      if ($options) {
        $query .= ' '. join(', ', $options);
      }
    }

    return $this->real_query($query);
  }

  /**
   * Changes the user of the specified database connection
   *
   * @param string $user - The MySQL user name.
   * @param string $password - The MySQL password.
   * @param string $database - The database to change to.   If desired,
   *   the NULL value may be passed resulting in only changing the user and
   *   not selecting a database. To select a database in this case use the
   *   mysqli_select_db() function.
   *
   * @return bool -
   */
  <<__Native>>
  public function change_user(string $user, string $password,
                              string $database): mixed;

  /**
   * Returns the default character set for the database connection
   *
   * @return string - The default character set for the current
   *   connection
   */
  <<__Native>>
  public function character_set_name(): mixed;

  /**
   * Closes a previously opened database connection
   *
   * @return bool -
   */
  public function close(): bool {
    $conn = $this->__connection;
    if ($conn) {
      return mysql_close($conn);
    }

    return true;
  }

  private function __end_transaction(bool $commit, int $flags = 0,
                                     ?string $name = null): ?bool {
    $query = ($commit) ? 'COMMIT' : 'ROLLBACK';
    if ($name) {
      $query .= '/*'. $name .'*/';
    }

    if ($flags) {
      switch ($flags & (MYSQLI_TRANS_COR_AND_CHAIN |
                        MYSQLI_TRANS_COR_AND_NO_CHAIN)) {
        case MYSQLI_TRANS_COR_AND_CHAIN:
          $query .= ' AND CHAIN';
        case MYSQLI_TRANS_COR_AND_NO_CHAIN:
          $query .= ' AND NO CHAIN';
        default:
          // Do nothing to mimic Zend
          break;
      }
      switch ($flags & (MYSQLI_TRANS_COR_RELEASE |
                        MYSQLI_TRANS_COR_NO_RELEASE)) {
        case MYSQLI_TRANS_COR_RELEASE:
          $query .= ' RELEASE';
        case MYSQLI_TRANS_COR_NO_RELEASE:
          $query .= ' AND NO RELEASE';
        default:
          // Do nothing to mimic Zend
          break;
      }
    }

    return $this->real_query($query);
  }

  /**
   * Commits the current transaction
   *
   * @param int $flags - A bitmask of MYSQLI_TRANS_COR_* constants.
   * @param string $name - If provided then COMMIT{name} is executed.
   *
   * @return bool -
   */
  public function commit(int $flags = 0, ?string $name = null): bool {
    return $this->__end_transaction(true, $flags, $name);
  }

  /**
   * Performs debugging operations
   *
   * @param string $message - A string representing the debugging
   *   operation to perform
   *
   * @return bool - Returns TRUE.
   */
  //public function debug(string $message): bool {
  //  return mysqli_debug($message);
  //}

  /**
   * Dump debugging information into the log
   *
   * @return bool -
   */
  <<__Native>>
  public function dump_debug_info(): mixed;

  /**
   * Alias for real_escape_string
   */
  public function escape_string($escapestr): ?string {
    return $this->real_escape_string($escapestr);
  }

 /**
   * Returns a character set object
   *
   * @return object - The function returns a character set object with the
   *   following properties:
   *   - charset Character set name
   *   - collation Collation name
   *   - dir Directory the charset description was fetched from (?) or "" for
   *         built-in character sets
   *   - min_length Minimum character length in bytes
   *   - max_length Maximum character length in bytes
   *   - number Internal character set number
   *   - state Character set status (?)
   */
  <<__Native>>
  public function get_charset(): mixed;

  /**
   * Get MySQL client info
   *
   * @return string - A string that represents the MySQL client library
   *   version
   */
  public function get_client_info(): string {
    return mysqli_get_client_info();
  }

  /**
   * Returns statistics about the client connection
   *
   * @return bool - Returns an array with connection stats if success,
   *   FALSE otherwise.
   */
  //<<__Native>>
  //public function get_connection_stats(): mixed;

  /**
   * Get result of SHOW WARNINGS
   *
   * @return mysqli_warning -
   */
  public function get_warnings(): mixed {
    if (!$this->warning_count) {
      return false;
    }

    $res = $this->query('SHOW WARNINGS');
    if (!$res) {
      return false;
    }

    $warnings = $res->fetch_all(MYSQLI_ASSOC);
    if (!$warnings) {
      return false;
    }

    return new mysqli_warning($warnings);
  }

  /**
   * Initializes MySQLi and returns a resource for use with
   * mysqli_real_connect()
   *
   * @return mysqli - Returns an object.
   */
  public function init(): mysqli {
    $this->hh_init();
    return $this;
  }

  /**
   * Asks the server to kill a MySQL thread
   *
   * @param int $processid -
   *
   * @return bool -
   */
  <<__Native>>
  public function kill(int $processid): mixed;

  /**
   * Check if there are any more query results from a multi query
   *
   * @return bool - Returns TRUE if one or more result sets are available
   *   from a previous call to mysqli_multi_query(), otherwise FALSE.
   */
  public function more_results(): mixed {
    $conn = $this->hh_get_connection(2);
    if (!$conn) {
      return null;
    }
    return mysql_more_results($conn);
  }

  /**
   * Performs a query on the database
   *
   * @param string $query - The query, as a string.   Data inside the
   *   query should be properly escaped.
   *
   * @return bool - Returns FALSE if the first statement failed. To
   *   retrieve subsequent errors from other statements you have to call
   *   mysqli_next_result() first.
   */
  public function multi_query(string $query): ?bool {
    $conn = $this->hh_get_connection(2);
    if (!$conn) {
      return null;
    }
    return mysql_multi_query($query, $conn);
  }

  /**
   * Prepare next result from multi_query
   *
   * @return bool -
   */
  public function next_result(): ?bool {
    $conn = $this->hh_get_connection(2);
    if (!$conn) {
      return null;
    }
    return !mysql_next_result($conn);
  }

  /**
   * Set options
   *
   * @param int $option - The option that you want to set. It can be one
   *   of the following values:  Valid options    Name Description
   *   MYSQLI_OPT_CONNECT_TIMEOUT connection timeout in seconds (supported
   *   on Windows with TCP/IP since PHP 5.3.1)   MYSQLI_OPT_LOCAL_INFILE
   *   enable/disable use of LOAD LOCAL INFILE   MYSQLI_INIT_COMMAND
   *   command to execute after when connecting to MySQL server
   *   MYSQLI_READ_DEFAULT_FILE  Read options from named option file
   *   instead of my.cnf    MYSQLI_READ_DEFAULT_GROUP  Read options from
   *   the named group from my.cnf or the file specified with
   *   MYSQL_READ_DEFAULT_FILE.    MYSQLI_SERVER_PUBLIC_KEY  RSA public key
   *   file used with the SHA-256 based authentication.
   * @param mixed $value - The value for the option.
   *
   * @return bool -
   */
  <<__Native>>
  public function options(int $option, mixed $value): mixed;

  /**
   * Pings a server connection, or tries to reconnect if the connection has
   * gone down
   *
   * @return bool -
   */
  public function ping(): ?bool {
    $conn = $this->hh_get_connection(2);
    if (!$conn) {
      return null;
    }
    return mysql_ping($conn);
  }

  /**
   * Poll connections
   *
   * @param array $read - List of connections to check for outstanding
   *   results that can be read.
   * @param array $error - List of connections on which an error occurred,
   *   for example, query failure or lost connection.
   * @param array $reject - List of connections rejected because no
   *   asynchronous query has been run on for which the function could poll
   *   results.
   * @param int $sec - Number of seconds to wait, must be non-negative.
   * @param int $usec - Number of microseconds to wait, must be
   *   non-negative.
   *
   * @return int - Returns number of ready connections upon success,
   *   FALSE otherwise.
   */
  //<<__Native>>
  //public static function poll(array &$read,
  //                            array &$error,
  //                            array &$reject,
  //                            int $sec,
  //                            int $usec): int;

  /**
   * Prepare an SQL statement for execution
   *
   * @param string $query - The query, as a string.    You should not add
   *   a terminating semicolon or \g to the statement.    This parameter
   *   can include one or more parameter markers in the SQL statement by
   *   embedding question mark (?) characters at the appropriate positions.
   *      The markers are legal only in certain places in SQL statements.
   *   For example, they are allowed in the VALUES() list of an INSERT
   *   statement (to specify column values for a row), or in a comparison
   *   with a column in a WHERE clause to specify a comparison value.
   *   However, they are not allowed for identifiers (such as table or
   *   column names), in the select list that names the columns to be
   *   returned by a SELECT statement, or to specify both operands of a
   *   binary operator such as the = equal sign. The latter restriction is
   *   necessary because it would be impossible to determine the parameter
   *   type. It's not allowed to compare marker with NULL by ? IS NULL too.
   *   In general, parameters are legal only in Data Manipulation Language
   *   (DML) statements, and not in Data Definition Language (DDL)
   *   statements.
   *
   * @return mysqli_stmt - mysqli_prepare() returns a statement object or
   *   FALSE if an error occurred.
   */
  public function prepare(string $query): mixed {
    $stmt = new mysqli_stmt($this);
    $prepared = $stmt->prepare($query);

    if (!$prepared) {
      // If we failed to prepare we need to move the error messages that are on
      // the mysqli_stmt object to the mysqli object otherwise the user will
      // never be able to get them.
      $this->hh_update_last_error($stmt);
      return false;
    }

    return $stmt;
  }

  <<__Native>>
  private function hh_update_last_error(mysqli_stmt $stmt): void;

  /**
   * Performs a query on the database
   *
   * @param string $query - The query string.   Data inside the query
   *   should be properly escaped.
   * @param int $resultmode - Either the constant MYSQLI_USE_RESULT or
   *   MYSQLI_STORE_RESULT depending on the desired behavior. By default,
   *   MYSQLI_STORE_RESULT is used.   If you use MYSQLI_USE_RESULT all
   *   subsequent calls will return error Commands out of sync unless you
   *   call mysqli_free_result()   With MYSQLI_ASYNC (available with
   *   mysqlnd), it is possible to perform query asynchronously.
   *   mysqli_poll() is then used to get results from such queries.
   *
   * @return mixed - Returns FALSE on failure. For successful SELECT,
   *   SHOW, DESCRIBE or EXPLAIN queries mysqli_query() will return a
   *   mysqli_result object. For other successful queries mysqli_query()
   *   will return TRUE.
   */
  public function query(string $query,
                        int $resultmode = MYSQLI_STORE_RESULT): ?mixed {
    if ($resultmode !== MYSQLI_STORE_RESULT &&
        $resultmode !== MYSQLI_USE_RESULT) {
      trigger_error("Invalid value for resultmode", E_WARNING);
      return false;
    }

    $result = $this->hh_real_query($query);
    if ($result === null) {
      return null;
    }

    if ($result == 2) {
      if ($resultmode == MYSQLI_STORE_RESULT) {
        return $this->store_result();
      } else {
        return $this->use_result();
      }
    }

    return $result !== 0;
  }

  /**
   * Opens a connection to a mysql server
   *
   * @param string $host - Can be either a host name or an IP address.
   *   Passing the NULL value or the string "localhost" to this parameter,
   *   the local host is assumed. When possible, pipes will be used instead
   *   of the TCP/IP protocol.
   * @param string $username - The MySQL user name.
   * @param string $passwd - If provided or NULL, the MySQL server will
   *   attempt to authenticate the user against those user records which
   *   have no password only. This allows one username to be used with
   *   different permissions (depending on if a password as provided or
   *   not).
   * @param string $dbname - If provided will specify the default
   *   database to be used when performing queries.
   * @param mixed $port - Specifies the port number to attempt to connect
   *   to the MySQL server.
   * @param string $socket - Specifies the socket or named pipe that
   *   should be used.    Specifying the socket parameter will not
   *   explicitly determine the type of connection to be used when
   *   connecting to the MySQL server. How the connection is made to the
   *   MySQL database is determined by the host parameter.
   * @param int $flags - With the parameter flags you can set different
   *   connection options:   Supported flags    Name Description
   *   MYSQLI_CLIENT_COMPRESS Use compression protocol
   *   MYSQLI_CLIENT_FOUND_ROWS return number of matched rows, not the
   *   number of affected rows   MYSQLI_CLIENT_IGNORE_SPACE Allow spaces
   *   after function names. Makes all function names reserved words.
   *   MYSQLI_CLIENT_INTERACTIVE  Allow interactive_timeout seconds
   *   (instead of wait_timeout seconds) of inactivity before closing the
   *   connection    MYSQLI_CLIENT_SSL Use SSL (encryption)       For
   *   security reasons the MULTI_STATEMENT flag is not supported in PHP.
   *   If you want to execute multiple queries use the mysqli_multi_query()
   *   function.
   *
   * @return bool -
   */
  public function real_connect(?string $host = null,
                               ?string $username = null,
                               ?string $passwd = null,
                               ?string $dbname = null,
                               mixed $port = null,
                               ?string $socket = null,
                               ?int $flags = 0): bool {

    // TODO: Fix this to use ZendParamMode when it is available
    // See D1359972 for context
    if (is_string($port)) {
      if (!ctype_digit($port)) {
        throw new Exception('Port is not numeric');
      };
      $port = (int) $port;
    }
    if ($port !== null && !is_int($port)) {
      throw new Exception('Port is not numeric');
    }

    $server = null;
    if ($host) {
      $server = $host;
      if ($port) {
        $server .= ':'. $port;
      }
    } else if ($socket) {
      $server = ':'. $socket;
    }

    $ret = $this->hh_real_connect($server, $username, $passwd, $dbname, $flags);

    if (!$ret) {
      self::$__connection_errno = mysql_errno();
      self::$__connection_error = mysql_error();
      return false;
    }

    self::$__connection_errno = 0;
    self::$__connection_error = null;

    return true;
  }

  <<__Native>>
  private function hh_real_connect(?string $server,
                                   ?string $username,
                                   ?string $passwd,
                                   ?string $dbname,
                                   ?int $flags): bool;

  /**
   * Escapes special characters in a string for use in an SQL statement,
   * taking into account the current charset of the connection
   *
   * @param string $escapestr - The string to be escaped.   Characters
   *   encoded are NUL (ASCII 0), \n, \r, \, ', ", and Control-Z.
   *
   * @return string - Returns an escaped string.
   */
  public function real_escape_string($escapestr): ?string {
    $conn = $this->hh_get_connection(2);
    if (!$conn) {
      return null;
    }
    return mysql_real_escape_string($escapestr, $conn);
  }

  /**
   * Execute an SQL query
   *
   * @param string $query - The query, as a string.   Data inside the
   *   query should be properly escaped.
   *
   * @return bool -
   */
  public function real_query(string $query): ?bool {
    $result = $this->hh_real_query($query);
    if ($result === null) {
      return null;
    }
    return $result !== 0;
  }

  <<__Native>>
  private function hh_real_query(string $query): ?int;

  /**
   * Get result from async query
   *
   * @return mysqli_result - Returns mysqli_result in success, FALSE
   *   otherwise.
   */
  //<<__Native>>
  //public function reap_async_query(): mysqli_result;

  /**
   * Refreshes
   *
   * @param int $options - The options to refresh, using the
   *   MYSQLI_REFRESH_* constants as documented within the MySQLi constants
   *   documentation.   See also the official MySQL Refresh documentation.
   *
   * @return bool - TRUE if the refresh was a success, otherwise FALSE
   */
  <<__Native>>
  public function refresh(int $options): mixed;

  /**
   * Rolls back a transaction to the named savepoint
   *
   * @param string $name -
   *
   * @return bool -
   */
  public function release_savepoint(string $name): ?bool {
    return $this->real_query('RELEASE SAVEPOINT `'. $name .'`');
  }

  /**
   * Rolls back current transaction
   *
   * @param int $flags - A bitmask of MYSQLI_TRANS_COR_* constants.
   * @param string $name - If provided then ROLLBACK{name} is executed.
   *
   * @return bool -
   */
  public function rollback(int $flags = 0, ?string $name = null): ?bool {
    return $this->__end_transaction(false, $flags, $name);
  }

  /**
   * Set a named transaction savepoint
   *
   * @param string $name -
   *
   * @return bool -
   */
  public function savepoint(string $name): ?bool {
    return $this->real_query('SAVEPOINT `'. $name .'`');
  }

  /**
   * Selects the default database for database queries
   *
   * @param string $dbname - The database name.
   *
   * @return bool -
   */
  public function select_db(string $dbname): ?bool {
    $conn = $this->hh_get_connection(2);
    if (!$conn) {
      return null;
    }
    return mysql_select_db($dbname, $conn);
  }

  /**
   * Sets the default client character set
   *
   * @param string $charset - The charset to be set as default.
   *
   * @return bool -
   */
  public function set_charset(string $charset) {
    $conn = $this->hh_get_connection(2);
    if (!$conn) {
      return null;
    }
    return mysql_set_charset($charset, $conn);
  }

  /**
   * Set callback function for LOAD DATA LOCAL INFILE command
   *
   * @param callable $read_func - A callback function or object method
   *   taking the following parameters:    stream A PHP stream associated
   *   with the SQL commands INFILE   buffer A string buffer to store the
   *   rewritten input into   buflen The maximum number of characters to be
   *   stored in the buffer   errormsg If an error occurs you can store an
   *   error message in here
   *
   * @return bool -
   */
  //<<__Native>>
  //public function set_local_infile_handler(callable $read_func): bool;

  /**
   * Alias of options()
   */
  public function set_opt(int $option, mixed $value): mixed {
    return $this->options($option, $value);
  }

  /**
   * Used for establishing secure connections using SSL
   *
   * @param string $key - The path name to the key file.
   * @param string $cert - The path name to the certificate file.
   * @param string $ca - The path name to the certificate authority file.
   * @param string $capath - The pathname to a directory that contains
   *   trusted SSL CA certificates in PEM format.
   * @param string $cipher - A list of allowable ciphers to use for SSL
   *   encryption.
   *
   * @return bool - This function always returns TRUE value. If SSL setup
   *   is incorrect mysqli_real_connect() will return an error when you
   *   attempt to connect.
   */
  <<__Native>>
  public function ssl_set(?string $key,
                          ?string $cert,
                          ?string $ca,
                          ?string $capath,
                          ?string $cipher): mixed;

  /**
   * Gets the current system status
   *
   * @return string - A string describing the server status. FALSE if an
   *   error occurred.
   */
  public function stat(): ?string {
    $conn = $this->hh_get_connection(2);
    if (!$conn) {
      return null;
    }
    return mysql_stat($conn);
  }

  /**
   * Initializes a statement and returns an object for use with
   * mysqli_stmt_prepare
   *
   * @return mysqli_stmt - Returns an object.
   */
  public function stmt_init(): mysqli_stmt {
    return new mysqli_stmt($this);
  }

  <<__Native>>
  private function hh_get_result(bool $use_store): ?mixed;

  /**
   * Transfers a result set from the last query
   *
   * @return mysqli_result - Returns a buffered result object or FALSE if
   *   an error occurred.    mysqli_store_result() returns FALSE in case
   *   the query didn't return a result set (if the query was, for example
   *   an INSERT statement). This function also returns FALSE if the
   *   reading of the result set failed. You can check if you have got an
   *   error by checking if mysqli_error() doesn't return an empty string,
   *   if mysqli_errno() returns a non zero value, or if
   *   mysqli_field_count() returns a non zero value. Also possible reason
   *   for this function returning FALSE after successful call to
   *   mysqli_query() can be too large result set (memory for it cannot be
   *   allocated). If mysqli_field_count() returns a non-zero value, the
   *   statement should have produced a non-empty result set.
   */
  public function store_result(): ?mixed {
    $result = $this->hh_get_result(true);
    if ($result === null) {
      return null;
    }
    if (is_bool($result)) {
      return false;
    }

    return new mysqli_result($result, MYSQLI_STORE_RESULT);
  }

  /**
   * Initiate a result set retrieval
   *
   * @return mysqli_result - Returns an unbuffered result object or FALSE
   *   if an error occurred.
   */
  public function use_result(): ?mixed {
    $result = $this->hh_get_result(false);
    if ($result === null) {
      return null;
    }
    if (is_bool($result)) {
      return $result;
    }

    return new mysqli_result($result, MYSQLI_USE_RESULT);
  }

}

/**
 * The mysqli exception handling class.
 */
//class mysqli_sql_exception {
//}

/**
 * MySQLi Driver.
 */
class mysqli_driver {

  private bool $__reconnect = false;
  private int $__report_mode = 0;

  public function __construct() {
    $this->__reconnect = ini_get("mysqli.reconnect") === "1" ? true : false;
  }

  public function __clone(): void {
    throw new Exception(
      'Trying to clone an uncloneable object of class mysqli_driver'
    );
  }
}

/**
 * Represents the result set obtained from a query against the database.
 *          5.4.0  Iterator support was added, as mysqli_result now implements
 * Traversable.
 */
class mysqli_result {

  // Not typing this since we are setting it as a mixed to comply with
  // GitHub issue 2082. As it is, even with invariants and various ifs
  // to guarantee the type, HHHBC is not able to infer the property type.
  // Anyway, the typehint is currently only for optimization purposes at this
  // point in time. See D1663326
  private $__result = null;
  private ?int $__resulttype = null;
  private bool $__done = false;

  <<__Native>>
  private function get_mysqli_conn_resource(mysqli $connection): ?resource;

  public function __construct(mixed $result,
                              int $resulttype = MYSQLI_STORE_RESULT) {
    if (!is_resource($result) && !($result instanceof mysqli)) {
      $msg = "Argument to mysqli_result::__construct must be of type "
           . "resource or mysqli";
      throw new Exception($msg);
    }
   if ($result instanceof mysqli) {
      $this->__result = $this->get_mysqli_conn_resource($result);
    } else {
      $this->__result = is_resource($result) ? $result : null;
    }
    $this->__resulttype = $resulttype;
  }

  public function __clone(): void {
    throw new Exception(
      'Trying to clone an uncloneable object of class mysqli_result'
    );
  }

  private function __checkRow(mixed $row) {
    if ($row == false) {
      $this->__done = true;
      return null;
    }
    return $row;
  }

  private function __mysqli_to_mysql_resulttype(int $resulttype): mixed {
    switch ($resulttype) {
      case MYSQLI_NUM:
        return MYSQL_NUM;
      case MYSQLI_ASSOC:
        return MYSQL_ASSOC;
      case MYSQLI_BOTH:
        return MYSQL_BOTH;
    }

    trigger_error('Mode can be only MYSQLI_NUM, MYSQLI_ASSOC or MYSQLI_BOTH',
                  E_WARNING);
    return null;
  }

  /**
   * Frees the memory associated with a result
   *
   * @return void -
   */
  public function close(): void {
    $this->free();
  }

  /**
   * Adjusts the result pointer to an arbitrary row in the result
   *
   * @param int $offset - The field offset. Must be between zero and the
   *   total number of rows minus one (0..mysqli_num_rows() - 1).
   *
   * @return bool -
   */
  public function data_seek(int $offset): mixed {
    if ($this->__result === null) {
      return null;
    }
    if ($this->__resulttype == MYSQLI_USE_RESULT) {
      return false;
    }

    return mysql_data_seek($this->__result, $offset);
  }

  /**
   * Fetches all result rows as an associative array, a numeric array, or
   * both
   *
   * @param int $resulttype - This optional parameter is a constant
   *   indicating what type of array should be produced from the current
   *   row data. The possible values for this parameter are the constants
   *   MYSQLI_ASSOC, MYSQLI_NUM, or MYSQLI_BOTH.
   *
   * @return mixed - Returns an array of associative or numeric arrays
   *   holding result rows.
   */
  public function fetch_all(int $resulttype = MYSQLI_NUM): mixed {
    $result = array();
    while (($row = $this->fetch_array($resulttype)) !== null) {
      $result[] = $row;
    }

    return $result;
  }

  /**
   * Fetch a result row as an associative, a numeric array, or both
   *
   * @param int $resulttype - This optional parameter is a constant
   *   indicating what type of array should be produced from the current
   *   row data. The possible values for this parameter are the constants
   *   MYSQLI_ASSOC, MYSQLI_NUM, or MYSQLI_BOTH.   By using the
   *   MYSQLI_ASSOC constant this function will behave identically to the
   *   mysqli_fetch_assoc(), while MYSQLI_NUM will behave identically to
   *   the mysqli_fetch_row() function. The final option MYSQLI_BOTH will
   *   create a single array with the attributes of both.
   *
   * @return mixed - Returns an array of strings that corresponds to the
   *   fetched row or NULL if there are no more rows in resultset.
   */
  public function fetch_array(int $resulttype = MYSQLI_BOTH): mixed {
    return $this->__checkRow(
      mysql_fetch_array(
        $this->__result,
        $this->__mysqli_to_mysql_resulttype($resulttype),
      )
    );
  }

  /**
   * Fetch a result row as an associative array
   *
   * @return array - Returns an associative array of strings representing
   *   the fetched row in the result set, where each key in the array
   *   represents the name of one of the result set's columns or NULL if
   *   there are no more rows in resultset.   If two or more columns of the
   *   result have the same field names, the last column will take
   *   precedence. To access the other column(s) of the same name, you
   *   either need to access the result with numeric indices by using
   *   mysqli_fetch_row() or add alias names.
   */
  public function fetch_assoc(): mixed {
    return $this->fetch_array(MYSQLI_ASSOC);
  }

  /**
   * Fetch meta-data for a single field
   *
   * @param int $fieldnr - The field number. This value must be in the
   *   range from 0 to number of fields - 1.
   *
   * @return object - Returns an object which contains field definition
   *   information or FALSE if no field information for specified fieldnr
   *   is available.    Object attributes    Attribute Description     name
   *   The name of the column   orgname Original column name if an alias
   *   was specified   table The name of the table this field belongs to
   *   (if not calculated)   orgtable Original table name if an alias was
   *   specified   def The default value for this field, represented as a
   *   string   max_length The maximum width of the field for the result
   *   set.   length The width of the field, as specified in the table
   *   definition.   charsetnr The character set number for the field.
   *   flags An integer representing the bit-flags for the field.   type
   *   The data type used for this field   decimals The number of decimals
   *   used (for integer fields)
   */
  public function fetch_field_direct(int $fieldnr): mixed {
    $this->field_seek($fieldnr);
    return $this->fetch_field();
  }

  /**
   * Returns the next field in the result set
   *
   * @return object - Returns an object which contains field definition
   *   information or FALSE if no field information is available.    Object
   *   properties    Property Description     name The name of the column
   *   orgname Original column name if an alias was specified   table The
   *   name of the table this field belongs to (if not calculated)
   *   orgtable Original table name if an alias was specified   def
   *   Reserved for default value, currently always ""   db Database (since
   *   PHP 5.3.6)   catalog The catalog name, always "def" (since PHP
   *   5.3.6)   max_length The maximum width of the field for the result
   *   set.   length The width of the field, as specified in the table
   *   definition.   charsetnr The character set number for the field.
   *   flags An integer representing the bit-flags for the field.   type
   *   The data type used for this field   decimals The number of decimals
   *   used (for integer fields)
   */
  <<__Native>>
  public function fetch_field(): mixed;

  /**
   * Returns an array of objects representing the fields in a result set
   *
   * @return array - Returns an array of objects which contains field
   *   definition information or FALSE if no field information is
   *   available.    Object properties    Property Description     name The
   *   name of the column   orgname Original column name if an alias was
   *   specified   table The name of the table this field belongs to (if
   *   not calculated)   orgtable Original table name if an alias was
   *   specified   max_length The maximum width of the field for the result
   *   set.   length The width of the field, as specified in the table
   *   definition.   charsetnr The character set number for the field.
   *   flags An integer representing the bit-flags for the field.   type
   *   The data type used for this field   decimals The number of decimals
   *   used (for integer fields)
   */
  public function fetch_fields(): array {
    $fields = array();
    $this->field_seek(0);
    while (($field = $this->fetch_field()) !== false) {
      $fields[] = $field;
    }

    return $fields;
  }

  /**
   * Returns the current row of a result set as an object
   *
   * @param string $class_name - The name of the class to instantiate,
   *   set the properties of and return. If not specified, a stdClass
   *   object is returned.
   * @param array $params - An optional array of parameters to pass to
   *   the constructor for class_name objects.
   *
   * @return object - Returns an object with string properties that
   *   corresponds to the fetched row or NULL if there are no more rows in
   *   resultset.
   */
  public function fetch_object(?string $class_name = null,
                               array $params = array()): mixed {
    if (func_num_args() == 0) {
      $obj = mysql_fetch_object($this->__result);
    } else {
      $obj = mysql_fetch_object($this->__result, $class_name, $params);
    }
    return $this->__checkRow($obj);
  }

  /**
   * Get a result row as an enumerated array
   *
   * @return mixed - mysqli_fetch_row() returns an array of strings that
   *   corresponds to the fetched row or NULL if there are no more rows in
   *   result set.
   */
  public function fetch_row(): mixed {
    return $this->fetch_array(MYSQLI_NUM);
  }

  /**
   * Set result pointer to a specified field offset
   *
   * @param int $fieldnr - The field number. This value must be in the
   *   range from 0 to number of fields - 1.
   *
   * @return bool -
   */
  public function field_seek(int $fieldnr): bool {
    if (!mysql_field_seek($this->__result, $fieldnr)) {
      return false;
    }

    return true;
  }

  /**
   * Frees the memory associated with a result
   *
   * @return void -
   */
  public function free(): void {
    mysql_free_result($this->__result);
    $this->__result = null;
  }

  /**
   * Frees the memory associated with a result
   *
   * @return void -
   */
  public function free_result(): void {
    $this->free();
  }

}

/**
 * Represents a prepared statement.
 */
class mysqli_stmt {

  private ?resource $__stmt = null;
  private ?mysqli $__link = null;

  public function __construct(mysqli $link, string $query = null) {
    $this->__link = $link;
    $this->hh_init($link);
    if ($query) {
      $this->prepare($query);
    }
  }

  public function __clone(): void {
    throw new Exception(
      'Trying to clone an uncloneable object of class mysqli_stmt'
    );
  }

  <<__Native>>
  private function hh_init(mysqli $connection): void;

  /**
   * Used to get the current value of a statement attribute
   *
   * @param int $attr - The attribute that you want to get.
   *
   * @return int - Returns FALSE if the attribute is not found, otherwise
   *   returns the value of the attribute.
   */
  <<__Native>>
  public function attr_get(int $attr): mixed;

  /**
   * Used to modify the behavior of a prepared statement
   *
   * @param int $attr - The attribute that you want to set. It can have
   *   one of the following values:  Attribute values    Character
   *   Description     MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH  If set to 1,
   *   causes mysqli_stmt_store_result() to update the metadata
   *   MYSQL_FIELD->max_length value.    MYSQLI_STMT_ATTR_CURSOR_TYPE  Type
   *   of cursor to open for statement when mysqli_stmt_execute() is
   *   invoked. mode can be MYSQLI_CURSOR_TYPE_NO_CURSOR (the default) or
   *   MYSQLI_CURSOR_TYPE_READ_ONLY.    MYSQLI_STMT_ATTR_PREFETCH_ROWS
   *   Number of rows to fetch from server at a time when using a cursor.
   *   mode can be in the range from 1 to the maximum value of unsigned
   *   long. The default is 1.        If you use the
   *   MYSQLI_STMT_ATTR_CURSOR_TYPE option with
   *   MYSQLI_CURSOR_TYPE_READ_ONLY, a cursor is opened for the statement
   *   when you invoke mysqli_stmt_execute(). If there is already an open
   *   cursor from a previous mysqli_stmt_execute() call, it closes the
   *   cursor before opening a new one. mysqli_stmt_reset() also closes any
   *   open cursor before preparing the statement for re-execution.
   *   mysqli_stmt_free_result() closes any open cursor.   If you open a
   *   cursor for a prepared statement, mysqli_stmt_store_result() is
   *   unnecessary.
   * @param int $mode - The value to assign to the attribute.
   *
   * @return bool -
   */
  <<__Native>>
  public function attr_set(int $attr, int $mode): mixed;

  /**
   * Binds variables to a prepared statement as parameters
   *
   * @param string $types - A string that contains one or more characters
   *   which specify the types for the corresponding bind variables:  Type
   *   specification chars    Character Description     i corresponding
   *   variable has type integer   d corresponding variable has type double
   *     s corresponding variable has type string   b corresponding
   *   variable is a blob and will be sent in packets
   * @param mixed $var1 - The number of variables and length of string
   *   types must match the parameters in the statement.
   * @param mixed ... -
   *
   * @return bool -
   */
  <<__Native("ActRec", "VariadicByRef")>>
  public function bind_param(string $types, ...): mixed;

  /**
   * Binds variables to a prepared statement for result storage
   *
   * @param mixed $var1 - The variable to be bound.
   * @param mixed ... -
   *
   * @return bool -
   */
  <<__Native("ActRec", "VariadicByRef")>>
  public function bind_result(...): mixed;

  /**
   * Closes a prepared statement
   *
   * @return bool -
   */
  <<__Native>>
  public function close(): mixed;

  /**
   * Seeks to an arbitrary row in statement result set
   *
   * @param int $offset - Must be between zero and the total number of
   *   rows minus one (0.. mysqli_stmt_num_rows() - 1).
   *
   * @return void -
   */
  <<__Native>>
  public function data_seek(int $offset): void;

  /**
   * Executes a prepared Query
   *
   * @return bool -
   */
  <<__Native>>
  public function execute(): mixed;

  /**
   * Fetch results from a prepared statement into the bound variables
   *
   * @return bool - Value Description     TRUE Success. Data has been
   *   fetched   FALSE Error occurred   NULL No more rows/data exists or
   *   data truncation occurred
   */
  <<__Native>>
  public function fetch(): mixed;

  /**
   * Frees stored result memory for the given statement handle
   *
   * @return void -
   */
  <<__Native>>
  public function free_result(): void;

  /**
   * Gets a result set from a prepared statement
   *
   * @return mysqli_result - Returns a resultset .
   */
  //public function get_result(): mysqli_result {
  //  return $this->__link->use_result();
  //}

  /**
   * Get result of SHOW WARNINGS
   *
   * @param mysqli_stmt $stmt -
   *
   * @return object -
   */
  public function get_warnings(): mixed {
    return $this->__link->get_warnings();
  }

  /**
   * Check if there are more query results from a multiple query
   *
   * @return bool - Returns TRUE if more results exist, otherwise FALSE.
   */
  //public function more_results(): bool {
  //  return $this->__link->more_results();
  //}

  /**
   * Reads the next result from a multiple query
   *
   * @return bool -
   */
  //public function next_result(): bool {
  //  return $this->__link->next_results();
  //}

  /**
   * Prepare an SQL statement for execution
   *
   * @param string $query - The query, as a string. It must consist of a
   *   single SQL statement.   You can include one or more parameter
   *   markers in the SQL statement by embedding question mark (?)
   *   characters at the appropriate positions.    You should not add a
   *   terminating semicolon or \g to the statement.     The markers are
   *   legal only in certain places in SQL statements. For example, they
   *   are allowed in the VALUES() list of an INSERT statement (to specify
   *   column values for a row), or in a comparison with a column in a
   *   WHERE clause to specify a comparison value.   However, they are not
   *   allowed for identifiers (such as table or column names), in the
   *   select list that names the columns to be returned by a SELECT
   *   statement), or to specify both operands of a binary operator such as
   *   the = equal sign. The latter restriction is necessary because it
   *   would be impossible to determine the parameter type. In general,
   *   parameters are legal only in Data Manipulation Language (DML)
   *   statements, and not in Data Definition Language (DDL) statements.
   *
   * @return mixed -
   */
  <<__Native>>
  public function prepare(string $query): mixed;

  /**
   * Resets a prepared statement
   *
   * @return bool -
   */
  <<__Native>>
  public function reset(): mixed;

  /**
   * Returns result set metadata from a prepared statement
   *
   * @return mysqli_result - Returns a result object or FALSE if an error
   *   occurred.
   */
  <<__Native>>
  public function result_metadata(): mixed;

  /**
   * Send data in blocks
   *
   * @param int $param_nr - Indicates which parameter to associate the
   *   data with. Parameters are numbered beginning with 0.
   * @param string $data - A string containing data to be sent.
   *
   * @return bool -
   */
  <<__Native>>
  public function send_long_data(int $param_nr, string $data): mixed;

  /**
   * Transfers a result set from a prepared statement
   *
   * @return bool -
   */
  public function store_result(): mixed {
    // First we need to set the MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH attribute in
    // some cases.
    $result = $this->result_metadata();
    $fields = $result->fetch_fields();
    foreach ($fields as $field) {
      if ($field->type == MYSQLI_TYPE_BLOB ||
          $field->type == MYSQLI_TYPE_MEDIUM_BLOB ||
          $field->type == MYSQLI_TYPE_LONG_BLOB ||
          $field->type == MYSQLI_TYPE_GEOMETRY) {
        $this->attr_set(MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH, 1);
        break;
      }
    }

    return $this->hh_store_result();
  }

  <<__Native>>
  public function hh_store_result(): mixed;

}

/**
 * Represents a MySQL warning.
 */
class mysqli_warning {

  public string $message;
  public string $sqlstate = "HY000";
  public int $errno;

  private $__warnings;

  /**
   * The __construct purpose
   *
   * @return  -
   */
  public function __construct(array $warnings): void {
    $this->__warnings = $warnings;
    $this->next();
  }

  /**
   * The next purpose
   *
   * @return bool -
   */
  public function next(): bool {
    if (!$this->__warnings) {
      return false;
    }

    $next = array_shift($this->__warnings);
    $this->message = $next['Message'];
    $this->errno = (int)$next['Code'];

    return true;
  }
}

/**
 * Returns client Zval cache statistics
 *
 * @return array - Returns an array with client Zval cache stats if
 *   success, FALSE otherwise.
 */
//function mysqli_get_cache_stats(): array {
//  throw NotSupportedException(__func__, "deprecated and removed");
//}

/**
 * Gets the number of affected rows in a previous MySQL operation
 *
 * @param mysqli $link -
 *
 * @return int - An integer greater than zero indicates the number of
 *   rows affected or retrieved. Zero indicates that no records were
 *   updated for an UPDATE statement, no rows matched the WHERE clause in
 *   the query or that no query has yet been executed. -1 indicates that
 *   the query returned an error.    If the number of affected rows is
 *   greater than the maximum integer value( PHP_INT_MAX ), the number of
 *   affected rows will be returned as a string.
 */
function mysqli_affected_rows(mysqli $link): ?int {
  return $link->affected_rows;
}

/**
 * Turns on or off auto-committing database modifications
 *
 * @param mysqli $link -
 * @param bool $mode - Whether to turn on auto-commit or not.
 *
 * @return bool -
 */
function mysqli_autocommit(mysqli $link, bool $mode): bool {
  return $link->autocommit($mode);
}

/**
 * Starts a transaction
 *
 * @param mysqli $link -
 * @param int $flags -
 * @param string $name -
 *
 * @return bool -
 */
function mysqli_begin_transaction(mysqli $link,
                                  int $flags = 0,
                                  ?string $name = null): bool {
  return $link->begin_transaction($flags, $name);
}

function mysqli_connect(?string $host = null,
                        ?string $username = null,
                        ?string $passwd = null,
                        ?string $dbname = null,
                        mixed $port = null,
                        ?string $socket = null): mixed {
  $link = new mysqli($host, $username, $passwd, $dbname, $port, $socket);
  if ($link->connect_errno > 0) {
    return false;
  }

  return $link;
}

/**
 * Changes the user of the specified database connection
 *
 * @param mysqli $link -
 * @param string $user - The MySQL user name.
 * @param string $password - The MySQL password.
 * @param string $database - The database to change to.   If desired, the
 *   NULL value may be passed resulting in only changing the user and not
 *   selecting a database. To select a database in this case use the
 *   mysqli_select_db() function.
 *
 * @return bool -
 */
function mysqli_change_user(mysqli $link,
                            string $user,
                            string $password,
                            string $database): bool {
  return $link->change_user($user, $password, $database);
}

/**
 * Returns the default character set for the database connection
 *
 * @param mysqli $link -
 *
 * @return string - The default character set for the current connection
 */
function mysqli_character_set_name(mysqli $link): string {
  return $link->character_set_name();
}

/**
 * Get MySQL client info
 *
 * @param mysqli $link -
 *
 * @return string - A string that represents the MySQL client library
 *   version
 */
<<__Native>>
function mysqli_get_client_info(): string;

/**
 * Returns the MySQL client version as an integer
 *
 * @param mysqli $link -
 *
 * @return int - A number that represents the MySQL client library
 *   version in format: main_version*10000 + minor_version *100 +
 *   sub_version. For example, 4.1.0 is returned as 40100.   This is useful
 *   to quickly determine the version of the client library to know if some
 *   capability exits.
 */
<<__Native>>
function mysqli_get_client_version(): int;

/**
 * Closes a previously opened database connection
 *
 * @param mysqli $link -
 *
 * @return bool -
 */
function mysqli_close(mysqli $link): bool {
  return $link->close();
}

/**
 * Commits the current transaction
 *
 * @param mysqli $link -
 * @param int $flags - A bitmask of MYSQLI_TRANS_COR_* constants.
 * @param string $name - If provided then COMMIT{name} is executed.
 *
 * @return bool -
 */
function mysqli_commit(mysqli $link, int $flags = 0,
                       ?string $name = null): bool {
  return $link->commit($flags, $name);
}

/**
 * Returns the error code from last connect call
 *
 * @return int - An error code value for the last call to
 *   mysqli_connect(), if it failed. zero means no error occurred.
 */
function mysqli_connect_errno(): int {
  return mysqli::$__connection_errno;
}

/**
 * Returns a string description of the last connect error
 *
 * @return string - A string that describes the error. NULL is returned
 *   if no error occurred.
 */
function mysqli_connect_error(): ?string {
  return mysqli::$__connection_error;
}

/**
 * Performs debugging operations
 *
 * @param string $message - A string representing the debugging operation
 *   to perform
 *
 * @return bool - Returns TRUE.
 */
//function mysqli_debug(string $message): bool {
//  throw 'NOT IMPLEMENTED';
//}

/**
 * Dump debugging information into the log
 *
 * @param mysqli $link -
 *
 * @return bool -
 */
function mysqli_dump_debug_info(mysqli $link): mixed {
  return $link->dump_debug_info();
}

/**
 * Returns the error code for the most recent function call
 *
 * @param mysqli $link -
 *
 * @return int - An error code value for the last call, if it failed.
 *   zero means no error occurred.
 */
function mysqli_errno(mysqli $link): ?int {
  return $link->errno;
}

/**
 * Returns a list of errors from the last command executed
 *
 * @param mysqli $link -
 *
 * @return array - A list of errors, each as an associative array
 *   containing the errno, error, and sqlstate.
 */
function mysqli_error_list(mysqli $link): array {
  return $link->error_list;
}

/**
 * Returns a string description of the last error
 *
 * @param mysqli $link -
 *
 * @return string - A string that describes the error. An empty string if
 *   no error occurred.
 */
function mysqli_error(mysqli $link): ?string {
  return $link->error;
}

/**
 * Returns the number of columns for the most recent query
 *
 * @param mysqli $link -
 *
 * @return int - An integer representing the number of fields in a result
 *   set.
 */
function mysqli_field_count(mysqli $link): ?int {
  return $link->field_count;
}

/**
 * Returns a character set object
 *
 * @param mysqli $link -
 *
 * @return object - The function returns a character set object with the
 *   following properties:
 *   - charset Character set name
 *   - collation Collation name
 *   - dir Directory the charset description was fetched from (?) or "" for
 *         built-in character sets
 *   - min_length Minimum character length in bytes
 *   - max_length Maximum character length in bytes
 *   - number Internal character set number
 *   - state Character set status (?)
 */
function mysqli_get_charset(mysqli $link): mixed {
  return $link->get_charset();
}

/**
 * Returns client per-process statistics
 *
 * @return array - Returns an array with client stats if success, FALSE
 *   otherwise.
 */
//<<__Native>>
//function mysqli_get_client_stats(): array;

/**
 * Returns statistics about the client connection
 *
 * @param mysqli $link -
 *
 * @return array - Returns an array with connection stats if success,
 *   FALSE otherwise.
 */
//function mysqli_get_connection_stats(mysqli $link): mixed {
//  return $link->get_connection_stats();
//}

/**
 * Returns a string representing the type of connection used
 *
 * @param mysqli $link -
 *
 * @return string - A character string representing the server hostname
 *   and the connection type.
 */
function mysqli_get_host_info(mysqli $link): ?string {
  return $link->host_info;
}

/**
 * Returns the version of the MySQL protocol used
 *
 * @param mysqli $link -
 *
 * @return int - Returns an integer representing the protocol version.
 */
function mysqli_get_proto_info(mysqli $link): int {
  return (int)$link->protocol_version;
}

/**
 * Returns the version of the MySQL server
 *
 * @param mysqli $link -
 *
 * @return string - A character string representing the server version.
 */
function mysqli_get_server_info(mysqli $link): ?string {
  return $link->server_info;
}

/**
 * Returns the version of the MySQL server as an integer
 *
 * @param mysqli $link -
 *
 * @return int - An integer representing the server version.   The form
 *   of this version number is main_version * 10000 + minor_version * 100 +
 *   sub_version (i.e. version 4.1.0 is 40100).
 */
function mysqli_get_server_version(mysqli $link): ?int {
  return $link->server_version;
}

/**
 * Get result of SHOW WARNINGS
 *
 * @param mysqli $link -
 *
 * @return mysqli_warning -
 */
function mysqli_get_warnings(mysqli $link): mixed {
  return $link->get_warnings();
}

/**
 * Retrieves information about the most recently executed query
 *
 * @param mysqli $link -
 *
 * @return string - A character string representing additional
 *   information about the most recently executed query.
 */
function mysqli_info(mysqli $link): ?string {
  return $link->info;
}

/**
 * Initializes MySQLi and returns a resource for use with
 * mysqli_real_connect()
 *
 * @return mysqli - Returns an object.
 */
function mysqli_init(): mysqli {
  return new mysqli();
}

/**
 * Returns the auto generated id used in the last query
 *
 * @param mysqli $link -
 *
 * @return mixed - The value of the AUTO_INCREMENT field that was updated
 *   by the previous query. Returns zero if there was no previous query on
 *   the connection or if the query did not update an AUTO_INCREMENT value.
 *      If the number is greater than maximal int value, mysqli_insert_id()
 *   will return a string.
 */
function mysqli_insert_id(mysqli $link): mixed {
  return $link->insert_id;
}

/**
 * Asks the server to kill a MySQL thread
 *
 * @param mysqli $link -
 * @param int $processid -
 *
 * @return bool -
 */
function mysqli_kill(mysqli $link, int $processid): bool {
  return $link->kill($processid);
}

/**
 * Check if there are any more query results from a multi query
 *
 * @param mysqli $link -
 *
 * @return bool - Returns TRUE if one or more result sets are available
 *   from a previous call to mysqli_multi_query(), otherwise FALSE.
 */
function mysqli_more_results(mysqli $link): bool {
  return $link->more_results();
}

/**
 * Performs a query on the database
 *
 * @param mysqli $link -
 * @param string $query - The query, as a string.   Data inside the query
 *   should be properly escaped.
 *
 * @return bool - Returns FALSE if the first statement failed. To
 *   retrieve subsequent errors from other statements you have to call
 *   mysqli_next_result() first.
 */
function mysqli_multi_query(mysqli $link, string $query): bool {
  return $link->multi_query($query);
}

/**
 * Prepare next result from multi_query
 *
 * @param mysqli $link -
 *
 * @return bool -
 */
function mysqli_next_result(mysqli $link): bool {
  return $link->next_result();
}

/**
 * Set options
 *
 * @param mysqli $link -
 * @param int $option - The option that you want to set. It can be one of
 *   the following values:  Valid options    Name Description
 *   MYSQLI_OPT_CONNECT_TIMEOUT connection timeout in seconds (supported on
 *   Windows with TCP/IP since PHP 5.3.1)   MYSQLI_OPT_LOCAL_INFILE
 *   enable/disable use of LOAD LOCAL INFILE   MYSQLI_INIT_COMMAND command
 *   to execute after when connecting to MySQL server
 *   MYSQLI_READ_DEFAULT_FILE  Read options from named option file instead
 *   of my.cnf    MYSQLI_READ_DEFAULT_GROUP  Read options from the named
 *   group from my.cnf or the file specified with MYSQL_READ_DEFAULT_FILE.
 *     MYSQLI_SERVER_PUBLIC_KEY  RSA public key file used with the SHA-256
 *   based authentication.
 * @param mixed $value - The value for the option.
 *
 * @return bool -
 */
function mysqli_options(mysqli $link, int $option, mixed $value): mixed {
  return $link->options($option, $value);
}

/**
 * Pings a server connection, or tries to reconnect if the connection has gone
 * down
 *
 * @param mysqli $link -
 *
 * @return bool -
 */
function mysqli_ping(mysqli $link): ?bool {
  return $link->ping();
}

/**
 * Poll connections
 *
 * @param array $read - List of connections to check for outstanding
 *   results that can be read.
 * @param array $error - List of connections on which an error occurred,
 *   for example, query failure or lost connection.
 * @param array $reject - List of connections rejected because no
 *   asynchronous query has been run on for which the function could poll
 *   results.
 * @param int $sec - Number of seconds to wait, must be non-negative.
 * @param int $usec - Number of microseconds to wait, must be
 *   non-negative.
 *
 * @return int - Returns number of ready connections upon success, FALSE
 *   otherwise.
 */
//function mysqli_poll(array &$read,
//                     array &$error,
//                     array &$reject,
//                     int $sec,
//                     int $usec): int {
//  return mysqli::poll($read, $error, $reject, $sec, $usec);
//}

/**
 * Prepare an SQL statement for execution
 *
 * @param mysqli $link -
 * @param string $query - The query, as a string.    You should not add a
 *   terminating semicolon or \g to the statement.    This parameter can
 *   include one or more parameter markers in the SQL statement by
 *   embedding question mark (?) characters at the appropriate positions.
 *    The markers are legal only in certain places in SQL statements. For
 *   example, they are allowed in the VALUES() list of an INSERT statement
 *   (to specify column values for a row), or in a comparison with a column
 *   in a WHERE clause to specify a comparison value.   However, they are
 *   not allowed for identifiers (such as table or column names), in the
 *   select list that names the columns to be returned by a SELECT
 *   statement, or to specify both operands of a binary operator such as
 *   the = equal sign. The latter restriction is necessary because it would
 *   be impossible to determine the parameter type. It's not allowed to
 *   compare marker with NULL by ? IS NULL too. In general, parameters are
 *   legal only in Data Manipulation Language (DML) statements, and not in
 *   Data Definition Language (DDL) statements.
 *
 * @return mysqli_stmt - mysqli_prepare() returns a statement object or
 *   FALSE if an error occurred.
 */
function mysqli_prepare(mysqli $link, string $query): mixed {
  return $link->prepare($query);
}

/**
 * Performs a query on the database
 *
 * @param mysqli $link -
 * @param string $query - The query string.   Data inside the query
 *   should be properly escaped.
 * @param int $resultmode - Either the constant MYSQLI_USE_RESULT or
 *   MYSQLI_STORE_RESULT depending on the desired behavior. By default,
 *   MYSQLI_STORE_RESULT is used.   If you use MYSQLI_USE_RESULT all
 *   subsequent calls will return error Commands out of sync unless you
 *   call mysqli_free_result()   With MYSQLI_ASYNC (available with
 *   mysqlnd), it is possible to perform query asynchronously.
 *   mysqli_poll() is then used to get results from such queries.
 *
 * @return mixed - Returns FALSE on failure. For successful SELECT, SHOW,
 *   DESCRIBE or EXPLAIN queries mysqli_query() will return a mysqli_result
 *   object. For other successful queries mysqli_query() will return TRUE.
 */
function mysqli_query(mysqli $link,
                      string $query,
                      int $resultmode = MYSQLI_STORE_RESULT): mixed {
  return $link->query($query, $resultmode);
}

/**
 * Opens a connection to a mysql server
 *
 * @param mysqli $link -
 * @param string $host - Can be either a host name or an IP address.
 *   Passing the NULL value or the string "localhost" to this parameter,
 *   the local host is assumed. When possible, pipes will be used instead
 *   of the TCP/IP protocol.
 * @param string $username - The MySQL user name.
 * @param string $passwd - If provided or NULL, the MySQL server will
 *   attempt to authenticate the user against those user records which have
 *   no password only. This allows one username to be used with different
 *   permissions (depending on if a password as provided or not).
 * @param string $dbname - If provided will specify the default database
 *   to be used when performing queries.
 * @param mixed $port - Specifies the port number to attempt to connect to
 *   the MySQL server.
 * @param string $socket - Specifies the socket or named pipe that should
 *   be used.    Specifying the socket parameter will not explicitly
 *   determine the type of connection to be used when connecting to the
 *   MySQL server. How the connection is made to the MySQL database is
 *   determined by the host parameter.
 * @param int $flags - With the parameter flags you can set different
 *   connection options:   Supported flags    Name Description
 *   MYSQLI_CLIENT_COMPRESS Use compression protocol
 *   MYSQLI_CLIENT_FOUND_ROWS return number of matched rows, not the number
 *   of affected rows   MYSQLI_CLIENT_IGNORE_SPACE Allow spaces after
 *   function names. Makes all function names reserved words.
 *   MYSQLI_CLIENT_INTERACTIVE  Allow interactive_timeout seconds (instead
 *   of wait_timeout seconds) of inactivity before closing the connection
 *    MYSQLI_CLIENT_SSL Use SSL (encryption)       For security reasons the
 *   MULTI_STATEMENT flag is not supported in PHP. If you want to execute
 *   multiple queries use the mysqli_multi_query() function.
 *
 * @return bool -
 */
function mysqli_real_connect(mysqli $link,
                             ?string $host = null,
                             ?string $username = null,
                             ?string $passwd = null,
                             ?string $dbname = null,
                             mixed $port = null,
                             ?string $socket = null,
                             ?int $flags = 0): bool {
  return $link->real_connect($host, $username, $passwd, $dbname, $port, $socket,
                             $flags);
}

/**
 * Alias of mysqli_real_escape_string
 */
function mysqli_escape_string(mysqli $link, $escapestr): ?string {
  return mysqli_real_escape_string($link, $escapestr);
}

/**
 * Escapes special characters in a string for use in an SQL statement, taking
 * into account the current charset of the connection
 *
 * @param mysqli $link -
 * @param string $escapestr - The string to be escaped.   Characters
 *   encoded are NUL (ASCII 0), \n, \r, \, ', ", and Control-Z.
 *
 * @return string - Returns an escaped string.
 */
function mysqli_real_escape_string(mysqli $link, $escapestr): ?string {
  return $link->real_escape_string($escapestr);
}

/**
 * Execute an SQL query
 *
 * @param mysqli $link -
 * @param string $query - The query, as a string.   Data inside the query
 *   should be properly escaped.
 *
 * @return bool -
 */
function mysqli_real_query(mysqli $link, string $query): bool {
  return $link->real_query($query);
}

/**
 * Get result from async query
 *
 * @param mysql $link -
 *
 * @return mysqli_result - Returns mysqli_result in success, FALSE
 *   otherwise.
 */
//function mysqli_reap_async_query(mysqli $link): mysqli_result {
//  return $link->reap_async_query();
//}

/**
 * Refreshes
 *
 * @param resource $link -
 * @param int $options - The options to refresh, using the
 *   MYSQLI_REFRESH_* constants as documented within the MySQLi constants
 *   documentation.   See also the official MySQL Refresh documentation.
 *
 * @return int - TRUE if the refresh was a success, otherwise FALSE
 */
function mysqli_refresh(mysqli $link, int $options): int {
  return $link->refresh($options);
}

/**
 * Rolls back a transaction to the named savepoint
 *
 * @param mysqli $link -
 * @param string $name -
 *
 * @return bool -
 */
function mysqli_release_savepoint(mysqli $link, string $name): bool {
  return $link->release_savepoint($name);
}

/**
 * Rolls back current transaction
 *
 * @param mysqli $link -
 * @param int $flags - A bitmask of MYSQLI_TRANS_COR_* constants.
 * @param string $name - If provided then ROLLBACK{name} is executed.
 *
 * @return bool -
 */
function mysqli_rollback(mysqli $link, int $flags = 0,
                         ?string $name = null): ?bool {
  return $link->rollback($flags, $name);
}

/**
 * Set a named transaction savepoint
 *
 * @param mysqli $link -
 * @param string $name -
 *
 * @return bool -
 */
function mysqli_savepoint(mysqli $link, string $name): bool {
  return $link->savepoint($name);
}

/**
 * Selects the default database for database queries
 *
 * @param mysqli $link -
 * @param string $dbname - The database name.
 *
 * @return bool -
 */
function mysqli_select_db(mysqli $link, string $dbname): ?bool {
  return $link->select_db($dbname);
}

/**
 * Send the query and return
 *
 * @param mysqli $link -
 * @param string $query -
 *
 * @return bool -
 */
//function mysqli_send_query(mysqli $link, string $query): bool {
//  return $link->send_query($query);
//}

/**
 * Sets the default client character set
 *
 * @param mysqli $link -
 * @param string $charset - The charset to be set as default.
 *
 * @return bool -
 */
function mysqli_set_charset(mysqli $link, string $charset): mixed {
  return $link->set_charset($charset);
}

/**
 * Unsets user defined handler for load local infile command
 *
 * @param mysqli $link -
 *
 * @return void -
 */
//<<__Native>>
//function mysqli_set_local_infile_default(mysqli $link): void;

/**
 * Set callback function for LOAD DATA LOCAL INFILE command
 *
 * @param mysqli $link -
 * @param callable $read_func - A callback function or object method
 *   taking the following parameters:    stream A PHP stream associated
 *   with the SQL commands INFILE   buffer A string buffer to store the
 *   rewritten input into   buflen The maximum number of characters to be
 *   stored in the buffer   errormsg If an error occurs you can store an
 *   error message in here
 *
 * @return bool -
 */
//function mysqli_set_local_infile_handler(mysqli $link,
//                                         callable $read_func): bool {
//  $link->set_local_infile_handler($read_func);
//}

/**
 * Returns the SQLSTATE error from previous MySQL operation
 *
 * @param mysqli $link -
 *
 * @return string - Returns a string containing the SQLSTATE error code
 *   for the last error. The error code consists of five characters.
 *   '00000' means no error.
 */
function mysqli_sqlstate(mysqli $link): ?string {
  return $link->sqlstate;
}

/**
 * Used for establishing secure connections using SSL
 *
 * @param mysqli $link -
 * @param string $key - The path name to the key file.
 * @param string $cert - The path name to the certificate file.
 * @param string $ca - The path name to the certificate authority file.
 * @param string $capath - The pathname to a directory that contains
 *   trusted SSL CA certificates in PEM format.
 * @param string $cipher - A list of allowable ciphers to use for SSL
 *   encryption.
 *
 * @return bool - This function always returns TRUE value. If SSL setup
 *   is incorrect mysqli_real_connect() will return an error when you
 *   attempt to connect.
 */
function mysqli_ssl_set(mysqli $link,
                        ?string $key,
                        ?string $cert,
                        ?string $ca,
                        ?string $capath,
                        ?string $cipher): bool {
  return $link->ssl_set($key, $cert, $ca, $capath, $cipher);
}

/**
 * Gets the current system status
 *
 * @param mysqli $link -
 *
 * @return string - A string describing the server status. FALSE if an
 *   error occurred.
 */
function mysqli_stat(mysqli $link): ?string {
  return $link->stat();
}

/**
 * Initializes a statement and returns an object for use with
 * mysqli_stmt_prepare
 *
 * @param mysqli $link -
 *
 * @return mysqli_stmt - Returns an object.
 */
function mysqli_stmt_init(mysqli $link): mysqli_stmt {
  return $link->stmt_init();
}

/**
 * Transfers a result set from the last query
 *
 * @param mysqli $link -
 *
 * @return mysqli_result - Returns a buffered result object or FALSE if
 *   an error occurred.    mysqli_store_result() returns FALSE in case the
 *   query didn't return a result set (if the query was, for example an
 *   INSERT statement). This function also returns FALSE if the reading of
 *   the result set failed. You can check if you have got an error by
 *   checking if mysqli_error() doesn't return an empty string, if
 *   mysqli_errno() returns a non zero value, or if mysqli_field_count()
 *   returns a non zero value. Also possible reason for this function
 *   returning FALSE after successful call to mysqli_query() can be too
 *   large result set (memory for it cannot be allocated). If
 *   mysqli_field_count() returns a non-zero value, the statement should
 *   have produced a non-empty result set.
 */
function mysqli_store_result(mysqli $link): mysqli_result {
  return $link->store_result();
}

/**
 * Returns the thread ID for the current connection
 *
 * @param mysqli $link -
 *
 * @return int - Returns the Thread ID for the current connection.
 */
function mysqli_thread_id(mysqli $link): ?int {
  return $link->thread_id;
}

/**
 * Returns whether thread safety is given or not
 *
 * @return bool - TRUE if the client library is thread-safe, otherwise
 *   FALSE.
 */
<<__Native>>
function mysqli_thread_safe(): bool;

/**
 * Initiate a result set retrieval
 *
 * @param mysqli $link -
 *
 * @return mysqli_result - Returns an unbuffered result object or FALSE
 *   if an error occurred.
 */
function mysqli_use_result(mysqli $link): mysqli_result {
  return $link->use_result();
}

/**
 * Returns the number of warnings from the last query for the given link
 *
 * @param mysqli $link -
 *
 * @return int - Number of warnings or zero if there are no warnings.
 */
function mysqli_warning_count(mysqli $link): ?int {
  return $link->warning_count;
}

/**
 * Enables or disables internal report functions
 *
 * @param int $flags - Supported flags    Name Description
 *   MYSQLI_REPORT_OFF Turns reporting off   MYSQLI_REPORT_ERROR Report
 *   errors from mysqli function calls   MYSQLI_REPORT_STRICT  Throw
 *   mysqli_sql_exception for errors instead of warnings
 *   MYSQLI_REPORT_INDEX Report if no index or bad index was used in a
 *   query   MYSQLI_REPORT_ALL Set all options (report all)
 *
 * @return bool -
 */
function mysqli_report(int $flags): bool {
  (new mysqli_driver())->report_mode = $flags;
  return true;
}

/**
 * Alias of mysqli_options
 */
function mysqli_set_opt(mysqli $link, int $option, mixed $value): mixed {
  return mysqli_options($link, $option, $value);
}

/**
 * Get current field offset of a result pointer
 *
 * @param mysqli_result $result -
 *
 * @return int - Returns current offset of field cursor.
 */
function mysqli_field_tell(mysqli_result $result): int {
  return $result->current_field;
}

/**
 * Adjusts the result pointer to an arbitrary row in the result
 *
 * @param mysqli_result $result -
 * @param int $offset - The field offset. Must be between zero and the
 *   total number of rows minus one (0..mysqli_num_rows() - 1).
 *
 * @return bool -
 */
function mysqli_data_seek(mysqli_result $result, int $offset): mixed {
  return $result->data_seek($offset);
}

/**
 * Fetches all result rows as an associative array, a numeric array, or both
 *
 * @param mysqli_result $result -
 * @param int $resulttype - This optional parameter is a constant
 *   indicating what type of array should be produced from the current row
 *   data. The possible values for this parameter are the constants
 *   MYSQLI_ASSOC, MYSQLI_NUM, or MYSQLI_BOTH.
 *
 * @return mixed - Returns an array of associative or numeric arrays
 *   holding result rows.
 */
function mysqli_fetch_all(mysqli_result $result,
                          int $resulttype = MYSQLI_NUM): mixed {
  return $result->fetch_all($resulttype);
}

/**
 * Fetch a result row as an associative, a numeric array, or both
 *
 * @param mysqli_result $result -
 * @param int $resulttype - This optional parameter is a constant
 *   indicating what type of array should be produced from the current row
 *   data. The possible values for this parameter are the constants
 *   MYSQLI_ASSOC, MYSQLI_NUM, or MYSQLI_BOTH.   By using the MYSQLI_ASSOC
 *   constant this function will behave identically to the
 *   mysqli_fetch_assoc(), while MYSQLI_NUM will behave identically to the
 *   mysqli_fetch_row() function. The final option MYSQLI_BOTH will create
 *   a single array with the attributes of both.
 *
 * @return mixed - Returns an array of strings that corresponds to the
 *   fetched row or NULL if there are no more rows in resultset.
 */
function mysqli_fetch_array(mysqli_result $result,
                            int $resulttype = MYSQLI_BOTH): mixed {
  return $result->fetch_array($resulttype);
}

/**
 * Fetch a result row as an associative array
 *
 * @param mysqli_result $result -
 *
 * @return array - Returns an associative array of strings representing
 *   the fetched row in the result set, where each key in the array
 *   represents the name of one of the result set's columns or NULL if
 *   there are no more rows in resultset.   If two or more columns of the
 *   result have the same field names, the last column will take
 *   precedence. To access the other column(s) of the same name, you either
 *   need to access the result with numeric indices by using
 *   mysqli_fetch_row() or add alias names.
 */
function mysqli_fetch_assoc(mysqli_result $result): mixed {
  return $result->fetch_assoc();
}

/**
 * Fetch meta-data for a single field
 *
 * @param mysqli_result $result -
 * @param int $fieldnr - The field number. This value must be in the
 *   range from 0 to number of fields - 1.
 *
 * @return object - Returns an object which contains field definition
 *   information or FALSE if no field information for specified fieldnr is
 *   available.    Object attributes    Attribute Description     name The
 *   name of the column   orgname Original column name if an alias was
 *   specified   table The name of the table this field belongs to (if not
 *   calculated)   orgtable Original table name if an alias was specified
 *   def The default value for this field, represented as a string
 *   max_length The maximum width of the field for the result set.   length
 *   The width of the field, as specified in the table definition.
 *   charsetnr The character set number for the field.   flags An integer
 *   representing the bit-flags for the field.   type The data type used
 *   for this field   decimals The number of decimals used (for integer
 *   fields)
 */
function mysqli_fetch_field_direct(mysqli_result $result,
                                   int $fieldnr): mixed {
  return $result->fetch_field_direct($fieldnr);
}

/**
 * Returns the next field in the result set
 *
 * @param mysqli_result $result -
 *
 * @return object - Returns an object which contains field definition
 *   information or FALSE if no field information is available.    Object
 *   properties    Property Description     name The name of the column
 *   orgname Original column name if an alias was specified   table The
 *   name of the table this field belongs to (if not calculated)   orgtable
 *   Original table name if an alias was specified   def Reserved for
 *   default value, currently always ""   db Database (since PHP 5.3.6)
 *   catalog The catalog name, always "def" (since PHP 5.3.6)   max_length
 *   The maximum width of the field for the result set.   length The width
 *   of the field, as specified in the table definition.   charsetnr The
 *   character set number for the field.   flags An integer representing
 *   the bit-flags for the field.   type The data type used for this field
 *    decimals The number of decimals used (for integer fields)
 */
function mysqli_fetch_field(mysqli_result $result): mixed {
  return $result->fetch_field();
}

/**
 * Returns an array of objects representing the fields in a result set
 *
 * @param mysqli_result $result -
 *
 * @return array - Returns an array of objects which contains field
 *   definition information or FALSE if no field information is available.
 *     Object properties    Property Description     name The name of the
 *   column   orgname Original column name if an alias was specified
 *   table The name of the table this field belongs to (if not calculated)
 *    orgtable Original table name if an alias was specified   max_length
 *   The maximum width of the field for the result set.   length The width
 *   of the field, as specified in the table definition.   charsetnr The
 *   character set number for the field.   flags An integer representing
 *   the bit-flags for the field.   type The data type used for this field
 *    decimals The number of decimals used (for integer fields)
 */
function mysqli_fetch_fields(mysqli_result $result): array {
  return $result->fetch_fields();
}

/**
 * Returns the current row of a result set as an object
 *
 * @param mysqli_result $result -
 * @param string $class_name - The name of the class to instantiate, set
 *   the properties of and return. If not specified, a stdClass object is
 *   returned.
 * @param array $params - An optional array of parameters to pass to the
 *   constructor for class_name objects.
 *
 * @return object - Returns an object with string properties that
 *   corresponds to the fetched row or NULL if there are no more rows in
 *   resultset.
 */
function mysqli_fetch_object(mysqli_result $result,
                             ?string $class_name = null,
                             ?array $params = array()): mixed {
  if (func_num_args() < 2) {
    return $result->fetch_object();
  }

  return $result->fetch_object($class_name, $params);
}

/**
 * Get a result row as an enumerated array
 *
 * @param mysqli_result $result -
 *
 * @return mixed - mysqli_fetch_row() returns an array of strings that
 *   corresponds to the fetched row or NULL if there are no more rows in
 *   result set.
 */
function mysqli_fetch_row(mysqli_result $result): mixed {
  return $result->fetch_row();
}

/**
 * Get the number of fields in a result
 *
 * @param mysqli_result $result -
 *
 * @return int - The number of fields from a result set.
 */
function mysqli_num_fields(mysqli_result $result): ?int {
  return $result->field_count;
}

/**
 * Set result pointer to a specified field offset
 *
 * @param mysqli_result $result -
 * @param int $fieldnr - The field number. This value must be in the
 *   range from 0 to number of fields - 1.
 *
 * @return bool -
 */
function mysqli_field_seek(mysqli_result $result,
                           int $fieldnr): bool {
  return $result->field_seek($fieldnr);
}

/**
 * Frees the memory associated with a result
 *
 * @param mysqli_result $result -
 *
 * @return void -
 */
<<__Native>>
function mysqli_free_result(mixed $result): void;

/**
 * Returns the lengths of the columns of the current row in the result set
 *
 * @param mysqli_result $result -
 *
 * @return array - An array of integers representing the size of each
 *   column (not including any terminating null characters). FALSE if an
 *   error occurred.   mysqli_fetch_lengths() is valid only for the current
 *   row of the result set. It returns FALSE if you call it before calling
 *   mysqli_fetch_row/array/object or after retrieving all rows in the
 *   result.
 */
function mysqli_fetch_lengths(mysqli_result $result): array {
  return $result->lengths;
}

/**
 * Gets the number of rows in a result
 *
 * @param mysqli_result $result -
 *
 * @return int - Returns number of rows in the result set.    If the
 *   number of rows is greater than PHP_INT_MAX, the number will be
 *   returned as a string.
 */
function mysqli_num_rows(mysqli_result $result): int {
  return $result->num_rows;
}

/**
 * Returns the total number of rows changed, deleted, or
 *   inserted by the last executed statement
 *
 *
 * @param mysqli_stmt $stmt -
 *
 * @return int - An integer greater than zero indicates the number of
 *   rows affected or retrieved. Zero indicates that no records where
 *   updated for an UPDATE/DELETE statement, no rows matched the WHERE
 *   clause in the query or that no query has yet been executed. -1
 *   indicates that the query has returned an error. NULL indicates an
 *   invalid argument was supplied to the function.    If the number of
 *   affected rows is greater than maximal PHP int value, the number of
 *   affected rows will be returned as a string value.
 */
function mysqli_stmt_affected_rows(mysqli_stmt $stmt): ?int {
  return $stmt->affected_rows;
}

/**
 * Used to get the current value of a statement attribute
 *
 * @param mysqli_stmt $stmt -
 * @param int $attr - The attribute that you want to get.
 *
 * @return int - Returns FALSE if the attribute is not found, otherwise
 *   returns the value of the attribute.
 */
function mysqli_stmt_attr_get(mysqli_stmt $stmt, int $attr): mixed {
  return $stmt->attr_get($attr);
}

/**
 * Used to modify the behavior of a prepared statement
 *
 * @param mysqli_stmt $stmt -
 * @param int $attr - The attribute that you want to set. It can have one
 *   of the following values:  Attribute values    Character Description
 *    MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH  If set to 1, causes
 *   mysqli_stmt_store_result() to update the metadata
 *   MYSQL_FIELD->max_length value.    MYSQLI_STMT_ATTR_CURSOR_TYPE  Type
 *   of cursor to open for statement when mysqli_stmt_execute() is invoked.
 *   mode can be MYSQLI_CURSOR_TYPE_NO_CURSOR (the default) or
 *   MYSQLI_CURSOR_TYPE_READ_ONLY.    MYSQLI_STMT_ATTR_PREFETCH_ROWS
 *   Number of rows to fetch from server at a time when using a cursor.
 *   mode can be in the range from 1 to the maximum value of unsigned long.
 *   The default is 1.        If you use the MYSQLI_STMT_ATTR_CURSOR_TYPE
 *   option with MYSQLI_CURSOR_TYPE_READ_ONLY, a cursor is opened for the
 *   statement when you invoke mysqli_stmt_execute(). If there is already
 *   an open cursor from a previous mysqli_stmt_execute() call, it closes
 *   the cursor before opening a new one. mysqli_stmt_reset() also closes
 *   any open cursor before preparing the statement for re-execution.
 *   mysqli_stmt_free_result() closes any open cursor.   If you open a
 *   cursor for a prepared statement, mysqli_stmt_store_result() is
 *   unnecessary.
 * @param int $mode - The value to assign to the attribute.
 *
 * @return bool -
 */
function mysqli_stmt_attr_set(mysqli_stmt $stmt, int $attr, int $mode): bool {
  return $stmt->attr_set($attr, $mode);
}

/**
 * Binds variables to a prepared statement as parameters
 *
 * @param mysqli_stmt $stmt -
 * @param string $types - A string that contains one or more characters
 *   which specify the types for the corresponding bind variables:  Type
 *   specification chars    Character Description     i corresponding
 *   variable has type integer   d corresponding variable has type double
 *   s corresponding variable has type string   b corresponding variable is
 *   a blob and will be sent in packets
 * @param mixed $var1 - The number of variables and length of string
 *   types must match the parameters in the statement.
 * @param mixed $... -
 *
 * @return bool -
 */
<<__Native("ActRec", "VariadicByRef")>>
function mysqli_stmt_bind_param(mysqli_stmt $stmt, string $types, ...): mixed;

/**
 * Binds variables to a prepared statement for result storage
 *
 * @param mysqli_stmt $stmt -
 * @param mixed $var1 - The variable to be bound.
 * @param mixed $... -
 *
 * @return bool -
 */
<<__Native("ActRec", "VariadicByRef")>>
function mysqli_stmt_bind_result(mysqli_stmt $stmt, ...): mixed;

/**
 * Closes a prepared statement
 *
 * @param mysqli_stmt $stmt -
 *
 * @return bool -
 */
function mysqli_stmt_close(mysqli_stmt $stmt): bool {
  return $stmt->close();
}

/**
 * Seeks to an arbitrary row in statement result set
 *
 * @param mysqli_stmt $stmt -
 * @param int $offset - Must be between zero and the total number of rows
 *   minus one (0.. mysqli_stmt_num_rows() - 1).
 *
 * @return void -
 */
function mysqli_stmt_data_seek(mysqli_stmt $stmt, int $offset): void {
  $stmt->data_seek($offset);
}

/**
 * Returns the error code for the most recent statement call
 *
 * @param mysqli_stmt $stmt -
 *
 * @return int - An error code value. Zero means no error occurred.
 */
function mysqli_stmt_errno(mysqli_stmt $stmt): ?int {
  return $stmt->errno;
}

/**
 * Returns a list of errors from the last statement executed
 *
 * @param mysqli_stmt $stmt -
 *
 * @return array - A list of errors, each as an associative array
 *   containing the errno, error, and sqlstate.
 */
function mysqli_stmt_error_list(mysqli_stmt $stmt): ?array {
  return $stmt->error_list;
}

/**
 * Returns a string description for last statement error
 *
 * @param mysqli_stmt $stmt -
 *
 * @return string - A string that describes the error. An empty string if
 *   no error occurred.
 */
function mysqli_stmt_error(mysqli_stmt $stmt): mixed {
  return $stmt->error;
}

/**
 * Executes a prepared Query
 *
 * @param mysqli_stmt $stmt -
 *
 * @return bool -
 */
function mysqli_stmt_execute(mysqli_stmt $stmt): bool {
  return $stmt->execute();
}

/**
 * Fetch results from a prepared statement into the bound variables
 *
 * @param mysqli_stmt $stmt -
 *
 * @return bool - Value Description     TRUE Success. Data has been
 *   fetched   FALSE Error occurred   NULL No more rows/data exists or data
 *   truncation occurred
 */
function mysqli_stmt_fetch(mysqli_stmt $stmt): mixed {
  return $stmt->fetch();
}

/**
 * Returns the number of field in the given statement
 *
 * @param mysqli_stmt $stmt -
 *
 * @return int -
 */
function mysqli_stmt_field_count(mysqli_stmt $stmt): ?int {
  return $stmt->field_count;
}

/**
 * Frees stored result memory for the given statement handle
 *
 * @param mysqli_stmt $stmt -
 *
 * @return void -
 */
function mysqli_stmt_free_result(mysqli_stmt $stmt): void {
  return $stmt->free_result();
}

/**
 * Gets a result set from a prepared statement
 *
 * @param mysqli_stmt $stmt -
 *
 * @return mysqli_result - Returns a resultset .
 */
//function mysqli_stmt_get_result(mysqli_stmt $stmt): mysqli_result {
//  return $stmt->get_result();
//}

/**
 * Get result of SHOW WARNINGS
 *
 * @param mysqli_stmt $stmt -
 *
 * @return object -
 */
function mysqli_stmt_get_warnings(mysqli_stmt $stmt): mixed {
  return $stmt->get_warnings();
}

/**
 * Get the ID generated from the previous INSERT operation
 *
 * @param mysqli_stmt $stmt -
 *
 * @return mixed -
 */
function mysqli_stmt_insert_id(mysqli_stmt $stmt): mixed {
  return $stmt->insert_id;
}

/**
 * Check if there are more query results from a multiple query
 *
 * @param mysqli_stmt $stmt -
 *
 * @return bool - Returns TRUE if more results exist, otherwise FALSE.
 */
function mysqli_stmt_more_results(mysqli_stmt $stmt): bool {
  return $stmt->more_results();
}

/**
 * Reads the next result from a multiple query
 *
 * @param mysqli_stmt $stmt -
 *
 * @return bool -
 */
function mysqli_stmt_next_result(mysqli_stmt $stmt): bool {
  return $stmt->next_result;
}

/**
 * Return the number of rows in statements result set
 *
 * @param mysqli_stmt $stmt -
 *
 * @return int - An integer representing the number of rows in result
 *   set.
 */
function mysqli_stmt_num_rows(mysqli_stmt $stmt): mixed {
  return $stmt->num_rows;
}

/**
 * Returns the number of parameter for the given statement
 *
 * @param mysqli_stmt $stmt -
 *
 * @return int - Returns an integer representing the number of
 *   parameters.
 */
function mysqli_stmt_param_count(mysqli_stmt $stmt): int {
  return $stmt->param_count;
}

/**
 * Prepare an SQL statement for execution
 *
 * @param mysqli_stmt $stmt -
 * @param string $query - The query, as a string. It must consist of a
 *   single SQL statement.   You can include one or more parameter markers
 *   in the SQL statement by embedding question mark (?) characters at the
 *   appropriate positions.    You should not add a terminating semicolon
 *   or \g to the statement.     The markers are legal only in certain
 *   places in SQL statements. For example, they are allowed in the
 *   VALUES() list of an INSERT statement (to specify column values for a
 *   row), or in a comparison with a column in a WHERE clause to specify a
 *   comparison value.   However, they are not allowed for identifiers
 *   (such as table or column names), in the select list that names the
 *   columns to be returned by a SELECT statement), or to specify both
 *   operands of a binary operator such as the = equal sign. The latter
 *   restriction is necessary because it would be impossible to determine
 *   the parameter type. In general, parameters are legal only in Data
 *   Manipulation Language (DML) statements, and not in Data Definition
 *   Language (DDL) statements.
 *
 * @return bool -
 */
function mysqli_stmt_prepare(mysqli_stmt $stmt, string $query): mixed {
  return $stmt->prepare($query);
}

/**
 * Resets a prepared statement
 *
 * @param mysqli_stmt $stmt -
 *
 * @return bool -
 */
function mysqli_stmt_reset(mysqli_stmt $stmt): bool {
  return $stmt->reset();
}

/**
 * Returns result set metadata from a prepared statement
 *
 * @param mysqli_stmt $stmt -
 *
 * @return mysqli_result - Returns a result object or FALSE if an error
 *   occurred.
 */
function mysqli_stmt_result_metadata(mysqli_stmt $stmt): mixed {
  return $stmt->result_metadata();
}

/**
 * Send data in blocks
 *
 * @param mysqli_stmt $stmt -
 * @param int $param_nr - Indicates which parameter to associate the data
 *   with. Parameters are numbered beginning with 0.
 * @param string $data - A string containing data to be sent.
 *
 * @return bool -
 */
function mysqli_stmt_send_long_data(mysqli_stmt $stmt,
                                    int $param_nr,
                                    string $data): mixed {
  return $stmt->send_long_data($param_nr, $data);
}

/**
 * Returns SQLSTATE error from previous statement operation
 *
 * @param mysqli_stmt $stmt -
 *
 * @return string - Returns a string containing the SQLSTATE error code
 *   for the last error. The error code consists of five characters.
 *   '00000' means no error.
 */
function mysqli_stmt_sqlstate(mysqli_stmt $stmt): ?string {
  return $stmt->sqlstate;
}

/**
 * Transfers a result set from a prepared statement
 *
 * @param mysqli_stmt $stmt -
 *
 * @return bool -
 */
function mysqli_stmt_store_result(mysqli_stmt $stmt): bool {
  return $stmt->store_result();
}
