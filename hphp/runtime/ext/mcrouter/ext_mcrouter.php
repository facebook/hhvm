<?hh

/**
 * Any failed MCRouter action will throw an
 * instance of MCRouterException
 */
class MCRouterException extends Exception {
  public function getKey(): string { return $this->key; }
  public function getOp(): int { return $this->op; }

  public function __construct(
    string $message,
    protected int $op = MCRouter::mc_op_unknown,
    int $reply = MCRouter::mc_res_unknown,
    protected string $key = "",
  ) {
    parent::__construct($message, $reply);
  }
}

/**
 * Thrown when unable to parse an option list
 */
class MCRouterOptionException extends Exception {
  public function getErrors(): varray<darray<string,string>> {
    return $this->errors;
  }

  public function __construct(protected varray<darray<string,string>> $errors) {
    parent::__construct("Failure parsing options", 0);
  }
}

<<__NativeData>>
class MCRouter {

  /**
   * Initialize an MCRouter handle
   *
   * See: https://github.com/facebook/mcrouter/wiki
   * See: https://github.com/facebook/mcrouter/blob/master/
   *                         mcrouter/mcrouter_options_list.h
   *
   * @array $options - MCRouter options
   *                   Must contain, at minimum, one of:
   (                     config_str or config_file
   * @string $pid - Persistent ID
   */
  <<__Native>>
  public function __construct(
    darray<string,mixed> $options,
    string $pid = "",
  ): void;

  /**
   * Simplified constructor
   *
   * @param Vector $servers - List of memcache servers to connect to
   * @return - Instance of MCRouter
   */
  public static function createSimple(ConstVector<string> $servers): MCRouter {
    $options = dict[
      'config_str' => json_encode(dict[
        'pools' => dict[
          'P' => dict[
            'servers' => $servers,
          ],
        ],
        'route' => 'PoolRoute|P',
      ]),
    ];

    return new MCRouter($options, implode(',', $servers));
  }

  /**
   * Store a value
   *
   * @param string $key - Name of the key to store
   * @param string $value - Datum to store
   * @param int $flags
   * @param int $expiration
   *
   * @throws On failure (e.g. if the value already exists)
   */
  <<__Native>>
  public function add(
    string $key,
    string $value,
    int $flags = 0,
    int $expiration = 0,
  ): Awaitable<void>;

  /**
   * Store a value (replacing if present)
   *
   * @param string $key - Name of the key to store
   * @param string $value - Datum to store
   * @param int $flags
   * @param int $expiration
   *
   * @throws On failure
   */
  <<__Native>>
  public function set(
    string $key,
    string $value,
    int $flags = 0,
    int $expiration = 0,
  ): Awaitable<void>;

  /**
   * Compare and set
   *
   * @param int $cas - CAS token as returned by getRecord()
   * @param string $key - Name of the key to store
   * @param string $value - Datum to store
   * @param int $expiration
   *
   * @throws On failure or mismatched CAS token
   */
  <<__Native>>
  public function cas(
    int $cas,
    string $key,
    string $value,
    int $expiration = 0,
  ): Awaitable<void>;

  /**
   * Store a value
   *
   * @param string $key - Name of the key to store
   * @param string $value - Datum to store
   * @param int $flags
   * @param int $expiration
   *
   * @throws On failure (e.g. No existing value to replace)
   */
  <<__Native>>
  public function replace(
    string $key,
    string $value,
    int $flags = 0,
    int $expiration = 0,
  ): Awaitable<void>;

  /**
   * Modify a value
   *
   * @param string $key - Name of the key to modify
   * @param string $value - String to prepend
   *
   * @throws On failure
   */
  <<__Native>>
  public function prepend(string $key, string $value): Awaitable<void>;

  /**
   * Modify a value
   *
   * @param string $key - Name of the key to modify
   * @param string $value - String to append
   *
   * @throws On failure
   */
  <<__Native>>
  public function append(string $key, string $value): Awaitable<void>;

  /**
   * Atomicly increment a numeric value
   *
   * @param string $key - Name of the key to modify
   * @param int $val - Amount to increment
   *
   * @return int - The new value
   * @throws On failure (e.g. $key not found)
   */
  <<__Native>>
  public function incr(string $key, int $val): Awaitable<int>;

  /**
   * Atomicly decrement a numeric value
   *
   * @param string $key - Name of the key to modify
   * @param int $val - Amount to decrement
   *
   * @return int - The new value
   * @throws On failure (e.g. $key not found)
   */
  <<__Native>>
  public function decr(string $key, int $val): Awaitable<int>;

  /**
   * Delete a key
   *
   * @param string $key - Key to delete
   *
   * @throws On failure
   */
  <<__Native>>
  public function del(string $key): Awaitable<void>;

  /**
   * Flush all key/value pairs
   *
   * @param int $delay - Amount of time to delay before flush
   *
   * @throws On failure
   */
  <<__Native>>
  public function flushAll(int $delay = 0): Awaitable<void>;

  /**
   * Retreive a value
   *
   * @param string $key - Name of the key to retreive
   *
   * @return string - The Value stored
   * @throws On failure
   */
  <<__Native>>
  public function get(string $key): Awaitable<string>;

  /**
   * Retreive a record and its metadata
   *
   * @param string $key = Name of the key to retreive
   *
   * @return array - Value retreived and additional metadata
   *   array(
   *     'value' => 'Value retreived',
   *     'cas'   => 1234567890,
   *     'flags' => 0x12345678,
   *   )
   * @throws On failure
   */
  <<__Native>>
  public function gets(string $key): Awaitable<darray>;

  /**
   * Get the remote server's current version
   *
   * @return string - The remote version
   *
   * @throws On failure
   */
  <<__Native>>
  public function version(): Awaitable<string>;

  /**
   * Translate an mc_op_* numeric code to something human-readable
   *
   * @parma int $op - One of the McRouterException::mc_op_* constants
   *                  Such as from MCRouterException::getOp()
   * @return string - The name of the op
   */
  <<__Native>>
  public static function getOpName(int $op): string;

  /**
   * Translate an mc_res_* numeric code to something human-readable
   *
   * @parma int $res - One of the McRouterException::mc_re_* constants
   *                   Such as from MCRouterException::getCode()
   * @return string - The name of the result
   */
  <<__Native>>
  public static function getResultName(int $op): string;
}
