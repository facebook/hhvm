<?hh // partial

class Redis {
  /* Redis servers run here by default */
  const DEFAULT_PORT = 6379;

  /* Return values from Redis::type() */
  const REDIS_NOT_FOUND = 0;
  const REDIS_STRING    = 1;
  const REDIS_SET       = 2;
  const REDIS_LIST      = 3;
  const REDIS_ZSET      = 4;
  const REDIS_HASH      = 5;

  /* Operational modes
   *
   * In ATOMIC mode, we wait for the server to
   *   respond to each request and return the value
   *   directly.
   *
   * In MULTI mode (activated by calling Redis::multi())
   *   we send commands immediately. but they are held in
   *   a transaction by the server.  Only upon calling
   *   Redis::exec() is the transaction committed, and the
   *   results returned.
   *
   * In PIPELINE mode (activated by calling Redis::pipeline())
   *   we queue all commands locally until invoking Redis::exec()
   *   at which point they are sent to the server in a single batch.
   *   And all results are packaged back in a single batch.
   *
   * In both MULTI and PIPELINE modes, pending commands may be
   *   discarded by calling Redis::discard()
   * The return value for both MULTI and PIPELINE for most commands
   *   is the object itself, meaning fluent calling may be used.
   */
  const ATOMIC   = 0;
  const MULTI    = 1;
  const PIPELINE = 2;

  /* Options to Redis::setOption() and Redis::getOption() */
  const OPT_SERIALIZER   = 1;
  const OPT_PREFIX       = 2;
  const OPT_READ_TIMEOUT = 3;
  const OPT_SCAN         = 4;

  /* Type of serialization to use with values stored in redis */
  const SERIALIZER_NONE     = 0;
  const SERIALIZER_PHP      = 1;

  /* Options used by lInsert and similar methods */
  const AFTER  = 'after';
  const BEFORE = 'before';

  /* Scan retry settings. We essentially always retry, so this is
     just for PHP 5 compatibility. */
  const SCAN_RETRY = 0;
  const SCAN_NORETRY = 1;

  /* Connection ---------------------------------------------------------- */

  public function connect($host,
                          $port = -1,
                          $timeout = 0.0,
                          $persistent_id = '',
                          $retry_interval = 0) {
    return $this->doConnect($host, $port, $timeout, $persistent_id,
                            $retry_interval, false);
  }

  public function pconnect($host,
                           $port = -1,
                           $timeout = 0.0,
                           $persistent_id = '',
                           $retry_interval = 0) {
    return $this->doConnect($host, $port, $timeout, $persistent_id,
                            $retry_interval, true);
  }

  public function auth($password) {
    $this->password = $password;
    $this->processCommand('AUTH', $password);
    return $this->processBooleanResponse();
  }

  public function close() {
    fclose($this->connection);
    $this->connection = null;
  }

  public function select($dbNumber) {
    $this->dbNumber = (int)$dbNumber;
    $this->processCommand("SELECT", (int)$dbNumber);
    return $this->processBooleanResponse();
  }

  public function setOption($opt, $value) {
    switch ($opt) {
      case self::OPT_PREFIX:
        $this->prefix = $value;
        return true;

      case self::OPT_SERIALIZER:
        if (($value !== self::SERIALIZER_NONE) &&
            ($value !== self::SERIALIZER_PHP)) {
          throw new RedisException("Invalid serializer option: $value");
        }
        $this->serializer = (int)$value;
        return true;

      case self::OPT_READ_TIMEOUT:
        $this->timeout_seconds  = (int)floor($value);
        $this->timeout_useconds =
          (int)(($value - $this->timeout_seconds) * 1000000);
        return stream_set_timeout($this->connection, $this->timeout_seconds,
                                                     $this->timeout_useconds);

      default:
        return false;
    }
  }

  public function getOption($opt) {
    switch ($opt) {
      case self::OPT_PREFIX:     return $this->prefix;
      case self::OPT_SERIALIZER: return $this->serializer;
      case self::OPT_READ_TIMEOUT:
        return $this->timeout_seconds + ($this->timeout_useconds / 1000000);
    }
    return false;
  }

  /* Server -------------------------------------------------------------- */

  public function config($op, $key, $val = '') {
    if ($op == 'GET') {
      $this->processCommand('CONFIG', 'GET', $key);
      return $this->processMapResponse(false, false);
    }
    if ($op == 'SET') {
      $this->processCommand('CONFIG', 'SET', $key, $val);
      return $this->processBooleanResponse();
    }
    throw new RedisException('First arg must be GET or SET');
  }

  public function info($option = '') {
    if ($option) {
      $this->processCommand('INFO', $option);
    } else {
      $this->processCommand('INFO');
    }
    return $this->processInfoResponse();
  }

  public function resetStat() {
    $this->processCommand('CONFIG', 'RESETSTAT');
    return $this->processBooleanResponse();
  }

  public function slaveOf($host = '', $port = -1) {
    if ($host) {
      if ($port <= 0) {
        $port = self::DEFAULT_PORT;
      }
      $this->processCommand('SLAVEOF', $host, (int)$port);
    } else {
      $this->processCommand('SLAVEOF', 'NO', 'ONE');
    }
    return $this->processBooleanResponse();
  }

  public function client($cmd, ...$args) {
    $cmd = strtolower($cmd);
    if ($args) {
      $this->processCommand('CLIENT', $cmd, $args[0]);
    } else {
      $this->processCommand('CLIENT', $cmd);
    }
    if ($cmd == 'list') {
      return $this->processClientListResponse();
    }
    return $this->processVariantResponse();
  }

  /* Strings ------------------------------------------------------------- */

  public function decr($key, $by = 1) {
    if ($by !== 1) {
      return $this->decrBy($key, $by);
    }
    $this->processCommand("DECR", $this->_prefix($key));
    return $this->processLongResponse();
  }

  public function decrBy($key, $by) {
    if ($by === 1) {
      return $this->decr($key);
    }
    $this->processCommand("DECRBY", $this->_prefix($key), (int)$by);
    return $this->processLongResponse();
  }

  public function incr($key, $by = 1) {
    if ($by !== 1) {
      return $this->incrBy($key, $by);
    }
    $this->processCommand("INCR", $this->_prefix($key));
    return $this->processLongResponse();
  }

  public function incrBy($key, $by) {
    if ($by === 1) {
      return $this->incr($key);
    }
    $this->processCommand("INCRBY", $this->_prefix($key), (int)$by);
    return $this->processLongResponse();
  }

  public function incrByFloat($key, $by) {
    $this->processCommand("INCRBYFLOAT", $this->_prefix($key),
                                               (float)$by);
    return $this->processDoubleResponse();
  }

  public function set($key, $value, $optionArrayOrExpiration = -1) {
    $key = $this->_prefix($key);
    $value = $this->_serialize($value);
    if (is_array($optionArrayOrExpiration) &&
        count($optionArrayOrExpiration) > 0) {
      $ex = array_key_exists('ex', $optionArrayOrExpiration);
      $px = array_key_exists('px', $optionArrayOrExpiration);
      $nx = in_array('nx', $optionArrayOrExpiration, true);
      $xx = in_array('xx', $optionArrayOrExpiration, true);
      if ($nx && $xx) {
        throw new RedisException(
          "Invalid set options: nx and xx may not be specified at the same time"
        );
      }
      $args = varray[
        $key,
        $value
      ];
      if ($px) {
        $args[] = "px";
        $args[] = $optionArrayOrExpiration['px'];
      } else if($ex) {
        $args[] = "ex";
        $args[] = $optionArrayOrExpiration['ex'];
      }
      if ($nx) {
        $args[] = "nx";
      } else if ($xx) {
        $args[] = "xx";
      }
      $this->processArrayCommand("SET", $args);
    } else if (is_numeric($optionArrayOrExpiration) &&
               (int)$optionArrayOrExpiration > 0) {
      $this->processCommand("SETEX", $key, $optionArrayOrExpiration, $value);
    } else {
      $this->processCommand("SET", $key, $value);
    }
    return $this->processBooleanResponse();
  }

  /* Keys ---------------------------------------------------------------- */

  public function sort($key, array $arr = null) {
    $using_store = false;
    $args = $this->sortClause($arr, inout $using_store);
    array_unshift(inout $args, $key);
    $this->processArrayCommand('SORT', $args);
    if ($using_store) {
      return $this->processVectorResponse(true);
    }
    return $this->processLongResponse();
  }

  public function sortAsc($key,
                          $pattern = null,
                          $get = null,
                          $start = -1,
                          $count = -1,
                          $store = null) {
    $limit = (($start > 0) && ($count > 0)) ? varray[$start, $count] : null;
    return $this->sort($key, darray[
      'by' => $pattern,
      'get' => $get,
      'limit' => $limit,
      'store' => $store,
      'dir' => 'ASC',
    ]);
  }

  public function sortAscAlpha($key,
                               $pattern = null,
                               $get = null,
                               $start = -1,
                               $count = -1,
                               $store = null) {
    $limit = (($start > 0) && ($count > 0)) ? varray[$start, $count] : null;
    return $this->sort($key, darray[
      'by' => $pattern,
      'get' => $get,
      'limit' => $limit,
      'store' => $store,
      'dir' => 'ASC',
      'alpha' => true,
    ]);
  }

  public function sortDesc($key,
                           $pattern = null,
                           $get = null,
                           $start = -1,
                           $count = -1,
                           $store = null) {
    $limit = (($start > 0) && ($count > 0)) ? varray[$start, $count] : null;
    return $this->sort($key, darray[
      'by' => $pattern,
      'get' => $get,
      'limit' => $limit,
      'store' => $store,
      'dir' => 'DESC',
    ]);
  }

  public function sortDescAlpha($key,
                                $pattern = null,
                                $get = null,
                                $start = -1,
                                $count = -1,
                                $store = null) {
    $limit = (($start > 0) && ($count > 0)) ? varray[$start, $count] : null;
    return $this->sort($key, darray[
      'by' => $pattern,
      'get' => $get,
      'limit' => $limit,
      'store' => $store,
      'dir' => 'DESC',
      'alpha' => true,
    ]);
  }

  public function object($info, $key) {
    $this->processCommand('OBJECT', $info, $this->_prefix($key));
    switch ($info) {
      case 'refcount': return $this->processLongResponse();
      case 'encoding': return $this->processStringResponse();
      default:         return $this->processBooleanResponse();
    }
  }

  /* Hashes -------------------------------------------------------------- */

  public function hMGet($key, array $members) {
    $members = array_values($members);
    $args = array_merge(varray[$this->_prefix($key)], $members);
    $this->processArrayCommand('HMGET', $args);
    return $this->processAssocResponse($members);
  }

  public function hMSet($key, array $pairs) {
    $args = varray[$this->_prefix($key)];
    foreach ($pairs as $k => $v) {
      $args[] = $k;
      $args[] = $this->_serialize($v);
    }
    $this->processArrayCommand('HMSET', $args);
    return $this->processBooleanResponse();
  }

  /* Sets ---------------------------------------------------------------- */

  public function sRandMember($key, $count = null) {
    $args = varray[$this->_prefix($key)];
    if ($count !== null) {
       $args[] = $count;
    }
    $this->processArrayCommand('SRANDMEMBER', $args);
    if ($count !== null) {
       return $this->processVectorResponse(true);
    }
    return $this->processStringResponse();
  }

  /* zSets --------------------------------------------------------------- */

  public function zAdd($key, $score, $value, ...$more_scores) {
    $args = varray[$key, $score, $value];
    if ($more_scores) {
      $args = array_merge($args, $more_scores);
    }

    $count = count($args);
    if ($count % 2 !== 1) {
      return false;
    }

    $args[0] = $this->_prefix($args[0]);
    for ($i = 1; $i < $count; $i += 2) {
      $args[$i  ] = (float)$args[$i];
      $args[$i+1] = $this->_serialize($args[$i+1]);
    }
    $this->processArrayCommand('ZADD', $args);
    return $this->processLongResponse();
  }

  protected function zInterUnionStore($cmd,
                                      $key,
                                      array $keys,
                                      array $weights = null,
                                      $op = '') {
    $args = varray[ $this->_prefix($key), count($keys) ];
    foreach ($keys as $k) {
      $args[] = $this->_prefix($k);
    }

    if ($weights) {
      $args[] = 'WEIGHTS';
      foreach ($weights as $weight) {
        if (is_int($weight) ||
            is_float($weight) ||
            ($weight ===  'inf') ||
            ($weight === '-inf') ||
            ($weight === '+inf')) {
          $args[] = $weight;
        }
      }
    }

    if ($op) {
      $args[] = 'AGGREGATE';
      $args[] = $op;
    }

    $this->processArrayCommand($cmd, $args);
    return $this->processLongResponse();
  }

  public function zInterStore($key,
                              array $keys,
                              array $weights = null,
                              $op = '') {
    return $this->zInterUnionStore('ZINTERSTORE', $key, $keys, $weights, $op);
  }

  public function zUnionStore($key,
                              array $keys,
                              array $weights = null,
                              $op = '') {
    return $this->zInterUnionStore('ZUNIONSTORE', $key, $keys, $weights, $op);
  }

  public function zRange($key, $start, $end, $withscores = false) {
    $args = varray[
      $this->_prefix($key),
      (int)$start,
      (int)$end,
    ];
    if ($withscores) {
      $args[] = 'WITHSCORES';
    }
    $this->processArrayCommand('ZRANGE', $args);
    if ($withscores) {
      return $this->processMapResponse(true, false);
    }
    return $this->processVectorResponse(true);
  }

  protected function zRangeByScoreImpl($cmd,
                                       $key,
                                       $start,
                                       $end,
                                       array $opts = null) {
    $args = varray[$this->_prefix($key), $start, $end];
    if (isset($opts['limit']) &&
        is_array($opts['limit']) &&
        (count($opts['limit']) == 2)) {
      list($limit_start, $limit_end) = $opts['limit'];
      $args[] = 'LIMIT';
      $args[] = $limit_start;
      $args[] = $limit_end;
    }
    if ($opts['withscores'] ?? false) {
      $args[] = 'WITHSCORES';
    }
    $this->processArrayCommand($cmd, $args);
    if ($opts['withscores'] ?? false) {
      return $this->processMapResponse(true, false);
    }
    return $this->processVectorResponse(true);
  }

  public function zRangeByScore($key, $start, $end, array $opts = null) {
    return $this->zRangeByScoreImpl('ZRANGEBYSCORE',
                                    $key, $start, $end, $opts);
  }

  public function zRevRangeByScore($key, $start, $end, array $opts = null) {
    return $this->zRangeByScoreImpl('ZREVRANGEBYSCORE',
                                    $key, $start, $end, $opts);
  }

  public function zRevRange($key, $start, $end, $withscores = false) {
    $args = varray[
      $this->_prefix($key),
      (int)$start,
      (int)$end,
    ];
    if ($withscores) {
      $args[] = 'WITHSCORES';
    }
    $this->processArrayCommand('ZREVRANGE', $args);
    if ($withscores) {
      return $this->processMapResponse(true, false);
    }
    return $this->processVectorResponse(true);
  }

  /* Scan --------------------------------------------------------------- */

  protected function scanImpl($cmd, $key, inout $cursor, $pattern, $count) {
    if ($this->mode != self::ATOMIC) {
      throw new RedisException("Can't call SCAN commands in multi or pipeline mode!");
    }

    $results = false;
    do {
      if ($cursor === 0) return $results;

      $args = varray[];
      if ($cmd !== 'SCAN') {
        $args[] = $this->_prefix($key);
      }
      $args[] = (int)$cursor;
      if ($pattern !== null) {
        $args[] = 'MATCH';
        $args[] = (string)$pattern;
      }
      if ($count !== null) {
        $args[] = 'COUNT';
        $args[] = (int)$count;
      }
      $this->processArrayCommand($cmd, $args);
      $resp = $this->processVariantResponse();
      if (!is_array($resp) || count($resp) != 2 || !is_array($resp[1])) {
        throw new RedisException(
          sprintf("Invalid %s response: %s", $cmd, print_r($resp, true)));
      }
      $cursor = (int)$resp[0];
      $results = $resp[1];
      // Provide SCAN_RETRY semantics by default. If iteration is done and
      // there were no results, $cursor === 0 check at the top of the loop
      // will pop us out.
    } while(count($results) == 0);
    return $results;
  }

  public function scan(inout $cursor, $pattern = null, $count = null) {
    return $this->scanImpl('SCAN', null, inout $cursor, $pattern, $count);
  }

  public function sScan($key, inout $cursor, $pattern = null, $count = null) {
    return $this->scanImpl('SSCAN', $key, inout $cursor, $pattern, $count);
  }

  public function hScan($key, inout $cursor, $pattern = null, $count = null) {
    $flat = $this->scanImpl('HSCAN', $key, inout $cursor, $pattern, $count);
    /*
     * HScan behaves differently from the other *scan functions. The wire
     * protocol returns names in even slots s, and the corresponding value
     * in odd slot s + 1. The PHP client returns these as an array mapping
     * keys to values.
     */
    if ($flat === false) return $flat;
    assert(count($flat) % 2 == 0);
    $ret = darray[];
    for ($i = 0; $i < count($flat); $i += 2) $ret[$flat[$i]] = $flat[$i + 1];
    return $ret;
  }

  public function zScan($key, inout $cursor, $pattern = null, $count = null) {
    $flat = $this->scanImpl('ZSCAN', $key, inout $cursor, $pattern, $count);
    if ($flat === false) return $flat;
    /*
     * ZScan behaves differently from the other *scan functions. The wire
     * protocol returns names in even slots s, and the corresponding value
     * in odd slot s + 1. The PHP client returns these as an array mapping
     * keys to values.
     */
    assert(count($flat) % 2 == 0);
    $ret = darray[];
    for ($i = 0; $i < count($flat); $i += 2) $ret[$flat[$i]] = $flat[$i + 1];
    return $ret;
  }

  /* Multi --------------------------------------------------------------- */

  protected function flushCallbacks($multibulk = true) {
    $type = null;
    if ($multibulk) $this->sockReadData(inout $type); // Response Count
    $ret = varray[];
    foreach ($this->multiHandler as $callback) {
      $args = isset($callback['args']) ? $callback['args'] : varray[];
      $ret[] = call_user_func_array($callback['cb'], $args);
    }
    $this->multiHandler = varray[];
    return $ret;
  }

  public function multi($mode = self::MULTI) {
    if ($mode === self::PIPELINE) {
      return $this->pipeline();
    }
    if ($mode !== self::MULTI) {
      return false;
    }
    $this->discard();
    $this->processCommand('MULTI');
    $type = null;
    $resp = $this->sockReadData(inout $type);
    if (($type === self::TYPE_LINE) && ($resp === 'OK')) {
      $this->mode = self::MULTI;
      return $this;
    }
    return false;
  }

  public function exec() {
    if ($this->mode === self::MULTI) {
      $this->mode = self::ATOMIC;
      $this->processCommand('EXEC');
      return $this->flushCallbacks();
    }
    if ($this->mode === self::PIPELINE) {
      $this->mode = self::ATOMIC;
      foreach ($this->commands as $cmd) {
        $this->processArrayCommand($cmd['cmd'], $cmd['args']);
      }
      $this->commands = varray[];
      return $this->flushCallbacks(false);
    }
  }

  public function discard() {
    $discard = ($this->mode === self::MULTI);
    $this->mode = self::ATOMIC;
    $this->commands = varray[];
    $this->multiHandler = varray[];
    if ($discard) {
       $this->processCommand('DISCARD');
       return $this->process1Response();
    }
    return true;
  }

  public function pipeline() {
    $this->discard();
    $this->mode = self::PIPELINE;
    return $this;
  }

  public function watch($key, ...$more_keys) {
    $keys = varray[$key];
    if ($more_keys) {
      $keys = array_merge($keys, $more_keys);
    }
    $args = array_map(($key) ==> $this->_prefix($key), $keys);
    $this->processArrayCommand("WATCH", $args);
    return $this->processBooleanResponse();
  }

  public function unwatch() {
    $this->processCommand("UNWATCH");
    return $this->processBooleanResponse();
  }

  /* Batch --------------------------------------------------------------- */

  protected function processMSetCommand($cmd, array $data) {
    $args = varray[];
    foreach ($data as $key => $val) {
      $args[] = $this->_prefix($key);
      $args[] = $this->_serialize($val);
    }
    $this->processArrayCommand($cmd, $args);
  }

  public function mSet(array $data) {
    $this->processMSetCommand('MSET', $data);
    return $this->processBooleanResponse();
  }

  public function mSetNx(array $data) {
    $this->processMSetCommand('MSETNX', $data);
    return $this->process1Response();
  }

  /* Scripting ----------------------------------------------------------- */

  protected function doEval($cmd, $script, array $args, $numKeys) {
    $keyCount = $numKeys;
    foreach($args as $idx => $arg) {
      if ($keyCount-- <= 0) break;
      $args[$idx] = $this->_prefix($arg);
    }
    array_unshift(inout $args, $numKeys);
    array_unshift(inout $args, $script);
    $this->processArrayCommand($cmd, $args);
    $response = $this->processVariantResponse();
    return ($response !== NULL) ? $response : false;
  }

  public function evaluate($script, array $args = varray[], $numKeys = 0) {
    return $this->doEval('EVAL', $script, $args, $numKeys);
  }

  public function eval($script, array $args = varray[], $numKeys = 0) {
    return $this->doEval('EVAL', $script, $args, $numKeys);
  }

  public function evaluateSha($sha, array $args = varray[], $numKeys = 0) {
    return $this->doEval('EVALSHA', $sha, $args, $numKeys);
  }

  public function evalSha($sha, array $args = varray[], $numKeys = 0) {
    return $this->doEval('EVALSHA', $sha, $args, $numKeys);
  }

  public function script($subcmd, ...$args) {
    switch (strtolower($subcmd)) {
      case 'flush':
      case 'kill':
        $this->processCommand('SCRIPT', $subcmd);
        $response = $this->processVariantResponse();
        return ($response !== NULL) ? true : false;
      case 'load':
        if (!$args) {
          return false;
        }
        $script = $args[0];
        if (!is_string($script) || !($script ?? false)) {
          return false;
        }
        $this->processCommand('SCRIPT', 'load', $script);
        $response = $this->processVariantResponse();
        return ($response !== NULL) ? $response : false;
      case 'exists':
        $this->processCommand('SCRIPT', 'EXISTS', ...$args);
        return $this->processVariantResponse();
      default:
        return false;
    }
  }

  /* Introspection ------------------------------------------------------- */

  public function isConnected() {
    return $this->checkConnection(false);
  }

  public function getHost() {
    return $this->host;
  }

  public function getPort() {
    return $this->port;
  }

  public function getDBNum() {
    return $this->dbNumber;
  }

  public function getTimeout() {
    return $this->timeout_connect;
  }

  public function getReadTimeout() {
    return $this->getOption(self::OPT_READ_TIMEOUT);
  }

  public function getPersistentId() {
    throw new RedisException('Named persistent connections are '.
                             'not supported.');
  }

  public function getPassword() {
    return $this->password;
  }

  public function getLastError() {
    return $this->lastError;
  }

  public function clearLastError() {
    $this->lastError = null;
    return true;
  }

  /* Standard Function Map ----------------------------------------------- */

  /**
   * The majority of the Redis API is implemented by __call
   * which references this list for how the individual command
   * should be handled.
   *
   * By default the name of the method (key in this array)
   * is uppercased to become the actual command sent to the redis server.
   *
   * Example: 'get' becomes the Redis command `GET`
   *
   * This mapping may be overridden by adding a 'cmd' element such as:
   *   'myget' => [ 'cmd' => 'GET' ],
   *
   * The argument spec is defined by the 'format' subparameter with each
   *   position in the string specifying what type of param it is.
   *  's' => Ordinary string to be passed trough to the server unmodified
   *  'k' => The name of a key.  Prefixed with $this->prefix.
   *  'v' => A piece of user data.  Serialized according to $this->serialize.
   *  'l' => An integer(long).  Explicitly cast from whatever is passed.
   *  'd' => A float(double).  Explicitly cast from whatever is passed.
   *  'b' => A boolean.  Explicitly cast from whatever is passed.
   *  'p' => Pivot point (self::BEFORE or self::AFTER).  Validated.
   *
   * In lieu of 'format', a mapping may specify 'vararg' for variadic methods.
   *   The value must be a bitmask of the VAR_* constants.
   *   See Redis::translateVarArgs()
   *
   * The method (on this class) called to handle the response is named by the
   *   'handler' field.  A shortcut for this is the 'return' field which will
   *   be mapped into 'handler' as: 'process{$return}Response'
   *   To pass arguments to the handler, use 'retargs'.
   *
   * Lastly, the 'alias' field (given by itself), will map calls from one
   *   function directly to another.  If the target method actually exists,
   *   the fcall will be proxied through call_user_func_array().  If the target
   *   is elsewhere in the map, __call's state will be reset to use the new
   *   map element.
   */
  protected static $map = darray[
    // Connection
    'open' => darray[ 'alias' => 'connect' ],
    'popen' => darray[ 'alias' => 'pconnect' ],
    'ping' => darray[ 'return' => 'Raw' ],
    'echo' => darray[ 'format' => 's', 'return' => 'String' ],
    'quit' => darray[ 'return' => 'Boolean' ],

    // Server
    'bgrewriteaof' => darray[ 'return' => 'Boolean' ],
    'bgsave' => darray[ 'return' => 'Boolean' ],
    'dbsize' => darray[ 'return' => 'Long' ],
    'flushall' => darray[ 'return' => 'Boolean' ],
    'flushdb' => darray[ 'return' => 'Boolean' ],
    'lastsave' => darray[ 'return' => 'Long' ],
    'save' => darray[ 'return' => 'Boolean' ],
    'time' => darray[ 'return' => 'Vector' ],

    // Strings
    'append' => darray[ 'format' => 'kv', 'return' => 'Long' ],
    'bitcount' => darray[ 'format' => 'kll', 'return' => 'Long' ],
    'bitop' => darray[ 'vararg' => self::VAR_KEY_NOT_FIRST, 'return' => 'Long' ],
    'get' => darray[ 'format' => 'k', 'return' => 'Serialized' ],
    'getbit' => darray[ 'format' => 'kl', 'return' => 'Long' ],
    'getrange' => darray[ 'format' => 'kll', 'return' => 'String', 'cmd' => 'RANGE' ],
    'getset' => darray[ 'format' => 'kv', 'return' => 'Serialized' ],
    'setbit' => darray[ 'format' => 'klv', 'return' => 'Long' ],
    'setex' => darray[ 'format' => 'klv', 'return' => 'Boolean' ],
    'psetex' => darray[ 'format' => 'klv', 'return' => 'Boolean' ],
    'setnx' => darray[ 'format' => 'kv', 'return' => '1' ],
    'setrange' => darray[ 'format' => 'klv', 'return' => 'Long' ],
    'strlen' => darray[ 'format' => 'k', 'return' => 'Long' ],

    // Keys
    'del' => darray[ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Long' ],
    'delete' => darray[ 'alias' => 'del' ],
    'dump' => darray[ 'format' => 'k', 'return' => 'Raw' ],
    'exists' => darray[ 'format' => 'k', 'return' => '1' ],
    'expire' => darray[ 'format' => 'kl', 'return' => '1' ],
    'settimeout' => darray[ 'alias' => 'expire' ],
    'pexpire' => darray[ 'format' => 'kl', 'return' => '1' ],
    'expireat' => darray[ 'format' => 'kl', 'return' => '1' ],
    'pexpireat' => darray[ 'format' => 'kl', 'return' => '1' ],
    'keys' => darray[ 'format' => 's', 'return' => 'Vector' ],
    'getkeys' => darray[ 'alias' => 'keys' ],
    'migrate' => darray[ 'format' => 'slkll', 'return' => 'Boolean' ],
    'move' => darray[ 'format' => 'kl', 'return' => '1' ],
    'persist' => darray[ 'format' => 'k', 'return' => '1' ],
    'randomkey' => darray[ 'return' => 'String' ],
    'rename' => darray[ 'format' => 'kk', 'return' => 'Boolean' ],
    'renamekey' => darray[ 'alias' => 'rename' ],
    'renamenx' => darray[ 'format' => 'kk', 'return' => '1' ],
    'type' => darray[ 'format' => 'k', 'return' => 'Type' ],
    'ttl' => darray[ 'format' => 'k', 'return' => 'Long' ],
    'pttl' => darray[ 'format' => 'k', 'return' => 'Long' ],
    'restore' => darray[ 'format' => 'kls', 'return' => 'Boolean' ],

    //Geospatial
    'geoadd' => darray[ 'format' => 'kdds', 'return' => 'Long' ],
    'geodist' => darray[ 'vararg' => self::VAR_KEY_FIRST, 'return' => 'String' ],
    'geohash' => darray[ 'vararg' => self::VAR_KEY_FIRST, 'return' => 'Vector' ],
    'geopos' => darray[ 'vararg' => self::VAR_KEY_FIRST, 'return' => 'Variant' ],
    'georadius' => darray[ 'vararg' => self::VAR_KEY_FIRST, 'return' => 'Variant' ],
    'georadiusbymember' => darray[ 'vararg' => self::VAR_KEY_FIRST, 'return' => 'Variant' ],

    // Hashes
    'hdel' => darray[ 'vararg' => self::VAR_KEY_FIRST, 'return' => 'Long' ],
    'hexists' => darray[ 'format' => 'ks', 'return' => '1' ],
    'hget' => darray[ 'format' => 'ks', 'return' => 'Serialized' ],
    'hgetall' => darray[ 'format' => 'k', 'return' => 'Map',
                                    'retargs' => varray[false,true] ],
    'hincrby' => darray[ 'format' => 'ksl', 'return' => 'Long' ],
    'hincrbyfloat' => darray[ 'format' => 'ksd', 'return' => 'Double' ],
    'hkeys' => darray[ 'format' => 'k', 'return' => 'Vector' ],
    'hlen' => darray[ 'format' => 'k', 'return' => 'Long' ],
    'hset' => darray[ 'format' => 'ksv', 'return' => 'Long' ],
    'hsetnx' => darray[ 'format' => 'ksv', 'return' => '1' ],
    'hvals' => darray[ 'format' => 'k', 'return' => 'Vector', 'retargs' => varray[1] ],

    // Lists
    'blpop' => darray[ 'vararg' => self::VAR_KEY_ALL_AND_TIMEOUT,
                 'return' => 'Vector', 'retargs' => varray[1] ],
    'brpop' => darray[ 'vararg' => self::VAR_KEY_ALL_AND_TIMEOUT,
                 'return' => 'Vector', 'retargs' => varray[1] ],
    'brpoplpush' => darray[ 'format' => 'kkl', 'return' => 'Serialized' ],
    'lindex' => darray[ 'format' => 'kl', 'return' => 'Serialized' ],
    'lget' => darray[ 'alias' => 'lindex' ],
    'linsert' => darray[ 'format' => 'kpkv', 'return' => 'Long' ],
    'llen' => darray[ 'format' => 'k', 'return' => 'Long', 'cmd' => 'LLEN' ],
    'lsize' => darray[ 'alias' => 'llen' ],
    'lpop' => darray[ 'format' => 'k', 'return' => 'Serialized' ],
    'lpush' => darray[ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                 'return' => 'Long' ],
    'lpushx' => darray[ 'format' => 'kl', 'return' => 'Long' ],
    'lrange' => darray[ 'format' => 'kll', 'return' => 'Vector', 'retargs' => varray[1] ],
    'lgetrange' => darray[ 'alias' => 'lrange' ],
    'lrem' => darray[ 'format' => 'kvs', 'return' => 'Long' ],
    'lremove' => darray[ 'alias' => 'lrem' ],
    'lset' => darray[ 'format' => 'klv', 'return' => 'Boolean' ],
    'ltrim' => darray[ 'format' => 'kll', 'return' => 'Boolean' ],
    'listtrim' => darray[ 'alias' => 'ltrim' ],
    'rpop' => darray[ 'format' => 'k', 'return' => 'Serialized' ],
    'rpoplpush' => darray[ 'format' => 'kk', 'return' => 'Serialized' ],
    'rpush' => darray[ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                 'return' => 'Long' ],
    'rpushx' => darray[ 'format' => 'kl', 'return' => 'Long' ],

    // Sets
    'sadd' => darray[ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                'return' => 'Long' ],
    'scard' => darray[ 'format' => 'k', 'return' => 'Long' ],
    'ssize' => darray[ 'alias' => 'scard' ],
    'sdiff' => darray[ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Vector' ],
    'sdiffstore' => darray[ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Long' ],
    'sinter' => darray[ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Vector' ],
    'sinterstore' => darray[ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Long' ],
    'sismember' => darray[ 'format' => 'kv', 'return' => '1' ],
    'scontains' => darray[ 'alias' => 'sismember' ],
    'smembers' => darray[ 'format' => 'k', 'return' => 'Vector' ],
    'sgetmembers' => darray[ 'alias' => 'smembers' ],
    'smove' => darray[ 'format' => 'kkv', 'return' => '1' ],
    'spop' => darray[ 'format' => 'k', 'return' => 'Serialized' ],
    'srem' => darray[ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                'return' => 'Long' ],
    'sremove' => darray[ 'alias' => 'srem' ],
    'sunion' => darray[ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Vector' ],
    'sunionstore' => darray[ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Long' ],

    // zSets
    'zcard' => darray[ 'format' => 'k', 'return' => 'Long' ],
    'zsize' => darray[ 'alias' => 'zcard' ],
    'zcount' => darray[ 'format' => 'kss', 'return' => 'Long' ],
    'zincrby' => darray[ 'format' => 'kdv', 'return' => 'Double' ],
    'zinter' => darray[ 'alias' => 'zinterstore' ],
    'zunion' => darray[ 'alias' => 'zunionstore' ],
    'zrank' => darray[ 'format' => 'kv', 'return' => 'Long' ],
    'zrevrank' => darray[ 'format' => 'kv', 'return' => 'Long' ],
    'zrem' => darray[ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                'return' => 'Long' ],
    'zremove' => darray[ 'alias' => 'zrem' ],
    'zdelete' => darray[ 'alias' => 'zrem' ],
    'zremrangebyrank' => darray[ 'format' => 'kll', 'return' => 'Long' ],
    'zdeleterangebyrank' => darray[ 'alias' => 'zremrangebyrank' ],
    'zremrangebyscore' => darray[ 'format' => 'kll', 'return' => 'Long' ],
    'zdeleterangebyscore' => darray[ 'alias' => 'zremrangebyscore' ],
    'zreverserange' => darray[ 'alias' => 'zrevrange' ],
    'zscore' => darray[ 'format' => 'kv', 'return' => 'Double' ],

    // Publish
    'publish' => darray[ 'format' => 'kv', 'return' => 'Long' ],
    /* These APIs are listed as "subject to change", avoid for now */
    'subscribe' => false,
    'psubscribe' => false,
    'unsubscribe' => false,
    'punsubscribe' => false,

    // Batch Ops
    'mget' => darray[ 'vararg' => self::VAR_KEY_ALL,
                'return' => 'Vector', 'retargs' => varray[1] ],
    'getmultiple' => darray[ 'alias' => 'mget' ],
  ];


  /* Internal Use Only beyond this point --------------------------------- */

  protected $host = '';
  protected $port = -1;
  protected $password = '';
  protected $dbNumber = 0;
  protected $last_connect = -1;
  protected $retry_interval = 0;
  protected $persistent = false;
  protected $connection = null;
  protected $lastError = null;

  protected $timeout_connect = 0;
  protected $timeout_seconds = 0;
  protected $timeout_useconds = 0;

  protected $mode = self::ATOMIC;
  protected $multiHandler = varray[];
  protected $commands = varray[];
  protected $prefix = '';
  protected $serializer = self::SERIALIZER_NONE;

  /* protocol ------------------------------------------------------------ */

  /* Internal use constants for var arg parsing */
  const VAR_KEY_NONE      = 0;
  const VAR_KEY_FIRST     = 1;
  const VAR_KEY_NOT_FIRST = 2;
  const VAR_KEY_ALL       = 3;
  const VAR_KEY_MASK      = 0x000F;

  const VAR_SERIALIZE     = 0x0010;
  const VAR_TIMEOUT       = 0x0020;

  const VAR_KEY_FIRST_AND_SERIALIZE = 0x0011;
  const VAR_KEY_ALL_AND_TIMEOUT     = 0x0023;

  /* Returned by reference from Redis::sockReadData()
   * Depending on the type of data returned by the server
   */
  const TYPE_LINE      = '+';
  const TYPE_INT       = ':';
  const TYPE_ERR       = '-';
  const TYPE_BULK      = '$';
  const TYPE_MULTIBULK = '*';

  protected function checkConnection($auto_reconnect = true) {
    if (!$this->connection) {
        return false;
    }

    // Check if we have hit the stream timeout
    if (stream_get_meta_data($this->connection)['timed_out']) {
      throw new RedisException("read error on connection");
    }
    if (!feof($this->connection)) {
      // Connection seems fine
      return true;
    }

    if ((time() - $this->last_connect) < $this->retry_interval) {
      // We've tried connecting too recently, don't retry
      return false;
    }

    if ($auto_reconnect &&
        $this->doConnect($this->host, $this->port,
                         $this->timeout_connect,
                         null, $this->retry_interval,
                         $this->persistent)) {
      if ($this->password) {
        $this->auth($this->password);
      }
      $this->select($this->dbNumber);
      return true;
    }

    // Reconnect failed, give up
    return false;
  }

  protected function sockReadLine() {
    $line = '';
    do {
      if (!$this->checkConnection()) {
        return false;
      }
      $line .= fgets($this->connection);
    } while (substr($line, -2) !== "\r\n");

    return substr($line, 0, -2);
  }

  protected function sockReadData(inout $type) {
    $line = $this->sockReadLine();
    if (strlen($line)) {
      $type = $line[0];
      $line = substr($line, 1);
      switch ($type) {
        case self::TYPE_ERR:
          if (!strncmp($line, '-ERR SYNC ', 10)) {
            throw new RedisException("Sync with master in progress");
          }
          if (!strncmp($line, 'OOM ', 4)) {
            throw new RedisException(
              "OOM command not allowed when used memory > 'maxmemory'");
          }
          if (!strncmp($line, 'EXECABORT ', 10)) {
            throw new RedisException(
              "Transaction discarded because of previous errors");
          }
          return $line;
        case self::TYPE_INT:
        case self::TYPE_LINE:
        case self::TYPE_MULTIBULK: // Count of elements to follow
          return $line;

        case self::TYPE_BULK:
          $bytes = (int)$line;
          if ($bytes < 0) return null;
          $buf = '';
          while (strlen($buf) < ($bytes + 2)) {
            $buf .= fread($this->connection, ($bytes + 2) - strlen($buf));
            if (!$this->checkConnection()) {
              return null;
            }
          }
          return substr($buf, 0, -2);

        default:
          throw new RedisException("protocol error, got '{$type}' ".
                                   "as reply type byte");
      }
    }
    return null;
  }

  /**
   * Process arguments for variadic functions based on $flags
   *
   * Redis::VAR_TIMEOUT indicates that the last argument
   *   in the list should be treated as an integer timeout
   *   for the operation
   * Redis::VAR_KEY_* indicates which (NONE, FIRST, NOT_FIRST, ALL)
   *   of the arguments (excluding TIMEOUT, as application)
   *   should be treated as keys, and thus prefixed with Redis::$prefix
   * Redis::VAR_SERIALIZE indicates that all non-timeout/non-key
   *   fields are data values, and should be serialzed
   *   (if a serialzied is specified)
   */
  protected function translateVarArgs(array $args, $flags) {
    // Check alternate vararg schemes first
    if (($flags & self::VAR_TIMEOUT) &&
        (count($args) == 2) &&
        (is_array($args[0])) &&
        (is_int($args[1]))) {
      $args = $args[0] + varray[$args[1]];
    }
    if ((!($flags & self::VAR_TIMEOUT)) &&
        (count($args) == 1) &&
        (is_array($args[0]))) {
      $args = $args[0];
    }

    // Then prefix, serialie, and cast as needed
    if ($flags & self::VAR_TIMEOUT) {
      $timeout = array_pop(inout $args);
    }
    if (($this->prefix && ($flags & self::VAR_KEY_MASK)) ||
        ($flags & self::VAR_SERIALIZE)) {
      $first = true;
      $varkey = $flags & self::VAR_KEY_MASK;
      foreach($args as $idx => $arg) {
        if (( $first && ($varkey == self::VAR_KEY_FIRST)) ||
            (!$first && ($varkey == self::VAR_KEY_NOT_FIRST)) ||
                         ($varkey == self::VAR_KEY_ALL)) {
          $args[$idx] = $this->_prefix($arg);
        } else if ($flags & self::VAR_SERIALIZE) {
          $args[$idx] = $this->_serialize($arg);
        }
        $first = false;
      }
    }
    if ($flags & self::VAR_TIMEOUT) {
      $args[] = (int)$timeout;
    }

    return $args;
  }

  /**
   * Actually send a command to the server.
   * assumes all appropriate prefixing and serialization
   * has been preformed by the caller and constructs
   * a Redis Protocol packet in the form:
   *
   * *N\r\n
   *
   * Folled by N instances of:
   *
   * $L\r\nA
   *
   * Where L is the length in bytes of argument A.
   *
   * So for the command `GET somekey` we'd serialize as:
   *
   * "*2\r\n$3\r\nGET\r\n$7\r\nsomekey\r\n"
   */
  protected function processArrayCommand($cmd, array $args) {
    if ($this->mode == self::PIPELINE) {
      $this->commands[] = darray[ 'cmd' => $cmd, 'args' => $args ];
      return true;
    }

    $clen = strlen($cmd);
    $count = count($args) + 1;
    $cmd = "*{$count}\r\n\${$clen}\r\n{$cmd}\r\n";

    while (count($args)) {
      $arg = (string)array_shift(inout $args);
      $alen = strlen($arg);
      $cmd .= "\${$alen}\r\n{$arg}\r\n";
    }

    if (!$this->checkConnection()) {
      return false;
    }
    return (bool)fwrite($this->connection, $cmd);
  }

  protected function processCommand($cmd, ...$args) {
    return $this->processArrayCommand($cmd, $args);
  }

  public function _serialize($str) {
    switch ($this->serializer) {
      case self::SERIALIZER_NONE:
        return $str;
      case self::SERIALIZER_PHP:
        return serialize($str);
      default:
        throw new RedisException("Not Implemented");
    }
  }

  public function _unserialize($str) {
    switch ($this->serializer) {
      case self::SERIALIZER_NONE:
        return $str;
      case self::SERIALIZER_PHP:
        return unserialize($str);
      default:
        throw new RedisException("Not Implemented");
    }
  }

  protected function processClientListResponse() {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = darray[ 'cb' => varray[$this,'processClientListResponse'] ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }
    $type = null;
    $resp = $this->sockReadData(inout $type);
    if (($type !== self::TYPE_LINE) && ($type !== self::TYPE_BULK)) {
      return null;
    }
    $ret = darray[];
    $pairs = explode(' ', trim($resp));
    foreach ($pairs as $pair) {
      $kv = explode('=', $pair, 2);
      if (count($kv) == 1) {
        $ret[] = $pair;
      } else {
        list($k, $v) = $kv;
        $ret[$k] = $v;
      }
    }
    return $ret;
  }

  protected function processVariantResponse() {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = darray[ 'cb' => varray[$this,'processVariantResponse'] ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }

    return $this->doProcessVariantResponse();
  }

  private function doProcessVariantResponse() {
    $type = null;
    $resp = $this->sockReadData(inout $type);

    if ($type === self::TYPE_INT) {
      return (int) $resp;
    }

    if ($type === self::TYPE_MULTIBULK) {
      if ($resp === '-1') {
          return '';
      }
      $ret = varray[];
      $lineNo = 0;
      $count = (int) $resp;
      while($count--) {
        $lineNo++;
        $ret[] = $this->doProcessVariantResponse();
      }
      return $ret;
    }

    if ($type === self::TYPE_ERR) {
      $this->lastError = $resp;
      return null;
    }

    return $resp;
  }

  protected function processSerializedResponse() {
    if ($this->mode === self::ATOMIC) {
      $type = null;
      $resp = $this->sockReadData(inout $type);
      if ($resp === null) {
        return false;
      }
      return (($type === self::TYPE_LINE) || ($type === self::TYPE_BULK))
             ? $this->_unserialize($resp) : false;
    }
    $this->multiHandler[] = darray[ 'cb' => varray[$this,'processSerializedResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processBooleanResponse() {
    if ($this->mode === self::ATOMIC) {
      $type = null;
      $resp = $this->sockReadData(inout $type);
      return ($type === self::TYPE_LINE) && ($resp === 'OK');
    }
    $this->multiHandler[] = darray[ 'cb' => varray[$this,'processBooleanResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processLongResponse() {
    if ($this->mode === self::ATOMIC) {
      $type = null;
      $resp = $this->sockReadData(inout $type);
      return ($type === self::TYPE_INT) ? ((int)$resp) : null;
    }
    $this->multiHandler[] = darray[ 'cb' => varray[$this,'processLongResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processDoubleResponse() {
    if ($this->mode === self::ATOMIC) {
      $type = null;
      $resp = $this->sockReadData(inout $type);
      if (($type === self::TYPE_INT) ||
          ($type === self::TYPE_BULK && is_numeric($resp))) {
        return (float)$resp;
      }
      return false;
    }
    $this->multiHandler[] = darray[ 'cb' => varray[$this,'processDoubleResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processStringResponse() {
    if ($this->mode === self::ATOMIC) {
      $type = null;
      $resp = $this->sockReadData(inout $type);
      return (($type === self::TYPE_LINE) || ($type === self::TYPE_BULK))
             ? ((string)$resp) : null;
    }
    $this->multiHandler[] = darray[ 'cb' => varray[$this,'processStringResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processVectorResponse($unser = 0) {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = darray[ 'cb' => varray[$this, 'processVectorResponse'],
                                'args' => varray[$unser]
                           ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }

    $type = null;
    $count = $this->sockReadData(inout $type);
    if ($type !== self::TYPE_MULTIBULK) {
      return null;
    }

    $ret = varray[];
    $lineNo = 0;
    while($count--) {
      $lineNo++;
      $type = null;
      $val = $this->sockReadData(inout $type);
      if ($unser && (($lineNo % $unser) == 0)) {
        $val = $this->_unserialize($val);
      }
      $ret[] = $val !== null ? $val : false;
    }
    return $ret;
  }

  protected function processMapResponse($unser_key, $unser_val = true) {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = darray[ 'cb' => varray[$this, 'processMapResponse'],
                                'args' => varray[$unser_key,$unser_val]
                              ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }

    $type = null;
    $count = $this->sockReadData(inout $type);
    if ($type !== self::TYPE_MULTIBULK) {
      return null;
    }

    $ret = darray[];
    while($count > 1) {
      $type = null;
      $key = $this->sockReadData(inout $type);
      if ($unser_key) {
        $key = $this->_unserialize($key);
      }
      $type = null;
      $val = $this->sockReadData(inout $type);
      if ($unser_val) {
        $val = $this->_unserialize($val);
      }
      $ret[$key] = $val;
      $count -= 2;
    }
    if ($count > 1) {
      $type = null;
      $ret[$this->sockReadData(inout $type)] = null;
    }
    return $ret;
  }

  protected function processAssocResponse(array $keys, $unser_val = true) {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = darray[ 'cb' => varray[$this, 'processAssocResponse'],
                                'args' => varray[$keys, $unser_val]
                              ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }

    $type = null;
    $count = $this->sockReadData(inout $type);
    if ($type !== self::TYPE_MULTIBULK) {
      return null;
    }

    $ret = darray[];
    while($count--) {
      $key = array_shift(inout $keys);
      $type = null;
      $val = $this->sockReadData(inout $type);
      if ($unser_val) {
        $val = $this->_unserialize($val);
      }
      $ret[$key] = $val !== null ? $val : false;
    }
    return $ret;
  }

  protected function process1Response() {
    if ($this->mode === self::ATOMIC) {
      $type = null;
      $resp = $this->sockReadData(inout $type);
      return ($type === self::TYPE_INT) && ($resp === '1');
    }
    $this->multiHandler[] = darray[ 'cb' => varray[$this,'process1Response'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processTypeResponse() {
    if ($this->mode === self::ATOMIC) {
      $type = null;
      $resp = $this->sockReadData(inout $type);
      if ($type !== self::TYPE_LINE) {
        return self::REDIS_NOT_FOUND;
      }
      switch($resp) {
        case 'string':  return self::REDIS_STRING;
        case 'set':     return self::REDIS_SET;
        case 'list':    return self::REDIS_LIST;
        case 'zset':    return self::REDIS_ZSET;
        case 'hash':    return self::REDIS_HASH;
        default:        return self::REDIS_NOT_FOUND;
      }
    }
    $this->multiHandler[] = darray[ 'cb' => 'processTypeResponse' ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processRawResponse() {
    if ($this->mode === self::ATOMIC) {
      return $this->sockReadLine();
    }
    $this->multiHandler[] = darray[ 'cb' => 'processRawResponse' ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processInfoResponse() {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = darray[ 'cb' => 'processInfoResponse' ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }
    $type = null;
    $resp = $this->sockReadData(inout $type);
    if (($type !== self::TYPE_LINE) && ($type !== self::TYPE_BULK)) {
      return false;
    }

    $ret = darray[];
    $lines = preg_split('/[\r\n]+/', $resp);
    foreach ($lines as $line) {
      if ((substr($line, 0, 1) == '#') ||
          !trim($line)) {
        continue;
      }
      $colon = strpos($line, ':');
      if ($colon === false) {
        break;
      }
      list($key, $val) = explode(':', $line, 2);
      $ret[$key] = $val;
    }
    return $ret;
  }

  protected function processQueuedResponse() {
    $type = null;
    $resp = $this->sockReadData(inout $type);
    return ($type === self::TYPE_LINE) && ($resp === 'QUEUED');
  }

  public function _prefix($key) {
    return $this->prefix . $key;
  }

  /**
   * Dispatches all commands in the Redis::$map list
   *
   * All other commands are handled by explicit implementations
   */
  public function __call($fname, $args) {
    if (ini_get('hhvm.no_use_magic_methods')) {
      trigger_error("Invoking Redis::$fname via magic __call", E_WARNING);
    }
    return $this->call__($fname, $args);
  }

  public function call__($fname, $args) {
    $fname = strtolower($fname);
    if (!isset(self::$map[$fname])) {
      trigger_error("Call to undefined function Redis::$fname()", E_ERROR);
      return null;
    }
    $func = self::$map[$fname];
    if ($func === false) {
      throw new RedisException("Redis::$fname() is currently unimplemented");
    }

    // Normalize record
    if ($func['alias'] ?? false) {
      if (isset(self::$map[$func['alias']])) {
        $fname = $func['alias'];
        $func = self::$map[$fname];
      } else {
        return call_user_func_array(varray[$this,$func['alias']],$args);
      }
    }
    if (!($func['format'] ?? false)) {
      $func['format'] = isset($func['vararg']) ? '...' : '';
    }
    if (!($func['cmd'] ?? false)) {
      $func['cmd'] = strtoupper($fname);
    }
    if (!($func['handler'] ?? false)) {
      $func['handler'] = !($func['return'] ?? false)
                       ? null : "process{$func['return']}Response";
    }
    if (!($func['retargs'] ?? false)) {
      $func['retargs'] = varray[];
    }

    $format = $func['format'];
    $argc = count($args);

    if ($format == '...') {
      $args = $this->translateVarArgs($args, $func['vararg']);
      $this->processArrayCommand($func['cmd'], $args);
      if (!($func['handler'] ?? false)) {
        return null;
      }
      return call_user_func_array(varray[$this, $func['handler']], $func['retargs']);
    }

    $flen = strlen($format);
    for ($i = 0; $i < $flen; $i++) {
      if (!array_key_exists($i, $args)) {
        if (isset($func['defaults']) &&
            array_key_exists($func['defaults'], $i)) {
          $args[$i] = $func['defaults'][$i];
        } else {
          trigger_error(
            "Redis::$fname requires at least $flen parameters $argc given",
            E_ERROR);
          return null;
        }
      }
      switch ($format[$i]) {
        case 'k': $args[$i] = $this->_prefix($args[$i]); break;
        case 'v': $args[$i] = $this->_serialize($args[$i]); break;
        case 's': $args[$i] = (string)$args[$i]; break;
        case 'l': $args[$i] = (int)$args[$i]; break;
        case 'd': $args[$i] = (float)$args[$i]; break;
        case 'b': $args[$i] = (bool)$args[$i]; break;
        case 'p':
          if (($args[$i] !== self::BEFORE) && ($args[$i] !== self::AFTER)) {
            trigger_error(
              "Argument $i to Redis::$fname must be 'before' or 'after'",
              E_ERROR);
            return null;
          } break;
      }
    }
    if ($func['cmd'] == "LREM") {
      //
      // The PHP interface has arguments in one order:
      //   https://github.com/nicolasff/phpredis#lrem-lremove
      // But the server wants them in another:
      //   http://redis.io/commands/lrem
      // So just swap them prior to marshalling them out.
      //
      $tmp = $args[1];
      $args[1] = $args[2];
      $args[2] = $tmp;
    }
    $this->processArrayCommand($func['cmd'], $args);
    if (!($func['handler'] ?? false)) {
      return null;
    }
    return call_user_func_array(varray[$this, $func['handler']], $func['retargs']);
  }

  /* --------------------------------------------------------------------- */

  protected function doConnect($host,
                               $port,
                               $timeout,
                               $persistent_id,
                               $retry_interval,
                               $persistent = false) {


    if ($port <= 0) {
      if ((strlen($host) > 0) && ($host[0] == '/')) {
        // Turn file path into unix:///path/to/sock
        $host = 'unix://' . $host;
        $port = 0;
      } elseif ((strlen($host) > 7) && !strncmp($host, 'unix://', 7)) {
        // Leave explicit unix:// socket as is
        $port = 0;
      } else {
        // Default port for TCP connections
        $port = self::DEFAULT_PORT;
      }
    }

    $errno = null;
    $errstr = null;
    if ($persistent) {
      if ($persistent_id ?? false) {
        $pid     = darray['id' => darray['persistent_id' => $persistent_id]];
        $context = stream_context_create($pid);
        $sok     = $host;
        if ($port > 0) $sok .= ':' . $port;
        $conn    = stream_socket_client(
          $sok, inout $errno, inout $errstr, $timeout, 2, $context);
      } else {
        $conn = pfsockopen($host, $port, inout $errno, inout $errstr, $timeout);
      }
    } else {
        $conn = fsockopen($host, $port, inout $errno, inout $errstr, $timeout);
    }
    $this->last_connect = time();
    $this->host = $host;
    $this->port = $port;
    $this->retry_interval = $retry_interval;
    $this->timeout_connect = $timeout;
    $this->persistent = $persistent;
    $this->persistent_id = $persistent_id;
    $this->connection = $conn;
    $this->commands = varray[];
    $this->multiHandler = varray[];
    $this->mode = self::ATOMIC;

    if (is_null($this->dbNumber)) {
      $this->dbNumber = 0;
    }

    if (!$this->connection) {
      trigger_error(
        "Failed connecting to redis server at {$host}: {$errstr}",
        E_WARNING);
      return false;
    }
    stream_set_blocking($this->connection, true);
    $this->setOption(Redis::OPT_READ_TIMEOUT, $timeout);

    return true;
  }

  protected function sortClause(array $arr, inout $using_store) {
    $using_store = false;
    if (!$arr) {
      return varray[];
    }

    $ret = varray[];
    foreach(varray['by','sort','store','get','alpha','limit','dir'] as $k) {
      if (isset($arr[$k])) {
        $v = $arr[$k];
      } else if (isset($arr[strtoupper($k)])) {
        $v = $arr[strtoupper($k)];
      } else {
        continue;
      }

      if (($k == 'get') && is_array($v)) {
        foreach ($v as $val) {
          $ret[] = 'GET';
          $ret[] = $val;
        }
        continue;
      }

      if ($k == 'alpha') {
        if ($v === true) {
          $ret[] = 'ALPHA';
        }
        continue;
      }

      if ($k == 'limit') {
        if (is_array($val) && (count($val) == 2)) {
          list($off, $cnt) = $val;
          $ret[] = 'LIMIT';
          $ret[] = $off;
          $ret[] = $cnt;
        }
        continue;
      }

      if ($k == 'store') {
        $using_store = true;
      }
      if ($k == 'dir') {
        $ret[] = strtoupper($v);
        continue;
      }

      $ret[] = strtoupper($k);
      $ret[] = $v;
    }

    return $ret;
  }
}
