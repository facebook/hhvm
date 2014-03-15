<?php

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

  /* Type of serialization to use with values stored in redis */
  const SERIALIZER_NONE     = 0;
  const SERIALIZER_PHP      = 1;
  const SERIALIZER_IGBINARY = 2;

  /* Options used by lInsert and similar methods */
  const AFTER  = 'after';
  const BEFORE = 'before';

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
    $this->processCommand('QUIT');
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
        if (($value !== self::SERIALIZER_NONE) AND
            ($value !== self::SERIALIZER_PHP) AND
            ($value !== self::SERIALIZER_IGBINARY)) {
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
    } else if ($op == 'SET') {
      $this->processCommand('CONFIG', 'SET', $key, $val);
      return $this->processBooleanResponse();
    } else {
      throw new RedisException('First arg must be GET or SET');
    }
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

  public function client($cmd, $arg = '') {
    if (func_num_args() == 2) {
      $this->processCommand('CLIENT', $cmd, $arg);
    } else {
      $this->processCommand('CLIENT', $cmd);
    }
    if ($cmd == 'list') {
      return $this->processClientListResponse();
    } else {
      return $this->processVariantResponse();
    }
  }

  /* Strings ------------------------------------------------------------- */

  public function decr($key, $by = 1) {
    if ($by !== 1) {
      return $this->decrBy($key, $by);
    }
    $this->processCommand("DECR", $this->prefix($key));
    return $this->processLongResponse();
  }

  public function decrBy($key, $by) {
    if ($by === 1) {
      return $this->decr($key);
    }
    $this->processCommand("DECRBY", $this->prefix($key), (int)$by);
    return $this->processLongResponse();
  }

  public function incr($key, $by = 1) {
    if ($by !== 1) {
      return $this->incrBy($key, $by);
    }
    $this->processCommand("INCR", $this->prefix($key));
    return $this->processLongResponse();
  }

  public function incrBy($key, $by) {
    if ($by === 1) {
      return $this->incr($key);
    }
    $this->processCommand("INCRBY", $this->prefix($key), (int)$by);
    return $this->processLongResponse();
  }

  public function incrByFloat($key, $by) {
    $this->processCommand("INCRBYFLOAT", $this->prefix($key),
                                               (float)$by);
    return $this->processDoubleResponse();
  }

  public function set($key, $value, $optionArrayOrExpiration = -1) {
    $key = $this->prefix($key);
    $value = $this->serialize($value);
    if (is_array($optionArrayOrExpiration) &&
        count($optionArrayOrExpiration) > 0) {
      $ex = array_key_exists('ex', $optionArrayOrExpiration);
      $px = array_key_exists('px', $optionArrayOrExpiration);
      $nx = in_array('nx', $optionArrayOrExpiration);
      $xx = in_array('xx', $optionArrayOrExpiration);
      if ($nx && $xx) {
        throw new RedisException(
          "Invalid set options: nx and xx may not be specified at the same time"
        );
      }
      $args = [
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
    $args = $this->sortClause($arr, $using_store);
    array_unshift($args, $key);
    $this->processArrayCommand('SORT', $args);
    if ($using_store) {
      return $this->processVectorResponse(true);
    } else {
      return $this->processLongResponse();
    }
  }

  public function sortAsc($key,
                          $pattern = null,
                          $get = null,
                          $start = -1,
                          $count = -1,
                          $store = null) {
    $limit = (($start > 0) AND ($count > 0)) ? [$start, $count] : null;
    return $this->sort($key, [
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
    $limit = (($start > 0) AND ($count > 0)) ? [$start, $count] : null;
    return $this->sort($key, [
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
    $limit = (($start > 0) AND ($count > 0)) ? [$start, $count] : null;
    return $this->sort($key, [
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
    $limit = (($start > 0) AND ($count > 0)) ? [$start, $count] : null;
    return $this->sort($key, [
      'by' => $pattern,
      'get' => $get,
      'limit' => $limit,
      'store' => $store,
      'dir' => 'DESC',
      'alpha' => true,
    ]);
  }

  public function object($info, $key) {
    $this->processCommand('OBJECT', $info, $this->prefix($key));
    switch ($info) {
      case 'refcount': return $this->processLongResponse();
      case 'encoding': return $this->processStringResponse();
      default:         return $this->processBooleanResponse();
    }
  }

  /* Hashes -------------------------------------------------------------- */

  public function hMGet($key, array $members) {
    $members = array_values($members);
    $args = array_merge([$this->prefix($key)], $members);
    $this->processArrayCommand('HMGET', $args);
    return $this->processAssocResponse($members);
  }

  public function hMSet($key, array $pairs) {
    $args = [$this->prefix($key)];
    foreach ($pairs as $k => $v) {
      $args[] = $k;
      $args[] = $this->serialize($v);
    }
    $this->processArrayCommand('HMSET', $args);
    return $this->processBooleanResponse();
  }

  /* zSets --------------------------------------------------------------- */

  public function zAdd($key, $score, $value/*, $scoreN, $valueN */) {
    $args = func_get_args();
    $count = count($args);
    if (($count - 1) % 2) {
      return false;
    }
    $args[0] = $this->prefix($args[0]);
    for ($i = 1; $i < $count; $i += 2) {
      $args[$i  ] = (double)$args[$i];
      $args[$i+1] = $this->serialize($args[$i+1]);
    }
    $this->processArrayCommand('ZADD', $args);
    return $this->processLongResponse();
  }

  protected function zInterUnionStore($cmd,
                                      $key,
                                      array $keys,
                                      array $weights = null,
                                      $op = '') {
    $args = [ $this->prefix($key), count($keys) ];
    foreach ($keys as $k) {
      $args[] = $this->prefix($k);
    }

    if ($weights) {
      $args[] = 'WEIGHTS';
      foreach ($weights as $weight) {
        if (is_int($weight) OR
            is_float($weight) OR
            ($weight ===  'inf') OR
            ($weight === '-inf') OR
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
    $args = [
      $this->prefix($key),
      (int)$start,
      (int)$end,
    ];
    if ($withscores) {
      $args[] = 'WITHSCORES';
    }
    $this->processArrayCommand('ZRANGE', $args);
    if ($withscores) {
      return $this->processMapResponse(true, false);
    } else {
      return $this->processVectorResponse(true);
    }
  }

  protected function zRangeByScoreImpl($cmd,
                                       $key,
                                       $start,
                                       $end,
                                       array $opts = null) {
    $args = [$this->prefix($key), (int)$start, (int)$end];
    if (isset($opts['limit']) AND
        is_array($opts['limit']) AND
        (count($opts['limit']) == 2)) {
      list($limit_start, $limit_end) = $opts['limit'];
      $args[] = 'LIMIT';
      $args[] = $limit_start;
      $args[] = $limit_end;
    }
    if (!empty($opts['withscores'])) {
      $args[] = 'WITHSCORES';
    }
    $this->processArrayCommand($cmd, $args);
    if (!empty($opts['withscores'])) {
      return $this->processMapResponse(true, false);
    } else {
      return $this->processVectorResponse(true);
    }
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
    $args = [
      $this->prefix($key),
      (int)$start,
      (int)$end,
    ];
    if ($withscores) {
      $args[] = 'WITHSCORES';
    }
    $this->processArrayCommand('ZREVRANGE', $args);
    if ($withscores) {
      return $this->processMapResponse(true, false);
    } else {
      return $this->processVectorResponse(true);
    }
  }

  /* Multi --------------------------------------------------------------- */

  protected function flushCallbacks($multibulk = true) {
    if ($multibulk) $this->sockReadData($type); // Response Count
    $ret = [];
    foreach ($this->multiHandler as $callback) {
      $args = isset($callback['args']) ? $callback['args'] : [];
      $ret[] = call_user_func_array($callback['cb'], $args);
    }
    $this->multiHandler = [];
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
    $resp = $this->sockReadData($type);
    if (($type === self::TYPE_LINE) AND ($resp === 'OK')) {
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
    } else if ($this->mode === self::PIPELINE) {
      $this->mode = self::ATOMIC;
      foreach ($this->commands as $cmd) {
        $this->processArrayCommand($cmd['cmd'], $cmd['args']);
      }
      $this->commands = [];
      return $this->flushCallbacks(false);
    }
  }

  public function discard() {
    $discard = ($this->mode === self::MULTI);
    $this->mode = self::ATOMIC;
    $this->commands = [];
    $this->multiHandler = [];
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

  public function watch($key, /* ... */) {
    $args = array_map([$this, 'prefix'], func_get_args());
    $this->processArrayCommand("WATCH", $args);
    return $this->processBooleanResponse();;
  }

  /* Batch --------------------------------------------------------------- */

  protected function processMSetCommand($cmd, array $data) {
    $args = [];
    foreach ($data as $key => $val) {
      $args[] = $this->prefix($key);
      $args[] = $this->serialize($val);
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
    foreach($args as &$arg) {
      if ($numKeys-- <= 0) break;
      $arg = $this->prefix($arg);
    }
    array_unshift($args, $script);
    $this->processArrayCommand($cmd, $args);
    return $this->processVariantResponse();
  }

  public function _eval($script, array $args = null, $numKeys = 0) {
    return $this->doEval('EVAL', $script, $args, $numKeys);
  }

  public function evalSha($sha, array $args = null, $numKeys = 0) {
    return $this->doEval('EVALSHA', $sha, $args, $numKeys);
  }

  public function script($subcmd/* ... */) {
    switch ($subcmd) {
      case 'flush':
      case 'kill':
        $this->processCommand('SCRIPT', $subcmd);
        return $this->processVariantResponse();
      case 'load':
        if (func_num_args() < 2) {
          return false;
        }
        $script = func_get_arg(1);
        if (!is_string($script) OR empty($script)) {
          return false;
        }
        $this->processCommand('SCRIPT', 'load', $script);
        return $this->processVariantResponse();
      case 'exists':
        $args = func_get_args();
        $args[0] = 'EXISTS';
        $this->processArrayCommand('SCRIPT', $args);
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
    return $this->getReadTimeout();
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
    $this->lastError = '';
    return true;
  }

  /* Standard Function Map ----------------------------------------------- */

  /**
   * The majority of the Redis API is implemnted by __call
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
  protected static $map = [
    // Connection
    'open' => [ 'alias' => 'connect' ],
    'popen' => [ 'alias' => 'pconnect' ],
    'ping' => [ 'return' => 'Raw' ],
    'echo' => [ 'format' => 's', 'return' => 'String' ],

    // Server
    'bgrewriteaof' => [ 'return' => 'Boolean' ],
    'bgsave' => [ 'return' => 'Boolean' ],
    'dbsize' => [ 'return' => 'Long' ],
    'flushall' => [ 'return' => 'Boolean' ],
    'flushdb' => [ 'return' => 'Boolean' ],
    'lastsave' => [ 'return' => 'Long' ],
    'save' => [ 'return' => 'Boolean' ],
    'time' => [ 'return' => 'Vector' ],

    // Strings
    'append' => [ 'format' => 'kv', 'return' => 'Long' ],
    'bitcount' => [ 'format' => 'kll', 'return' => 'Long' ],
    'bitop' => [ 'vararg' => self::VAR_KEY_NOT_FIRST, 'return' => 'Long' ],
    'get' => [ 'format' => 'k', 'return' => 'Serialized' ],
    'getbit' => [ 'format' => 'kl', 'return' => 'Long' ],
    'getrange' => [ 'format' => 'kll', 'return' => 'String', 'cmd' => 'RANGE' ],
    'getset' => [ 'format' => 'kv', 'return' => 'Serialized' ],
    'setbit' => [ 'format' => 'klv', 'return' => 'Long' ],
    'setex' => [ 'format' => 'klv', 'return' => 'Boolean' ],
    'psetex' => [ 'format' => 'klv', 'return' => 'Boolean' ],
    'setnx' => [ 'format' => 'kv', 'return' => '1' ],
    'setrange' => [ 'format' => 'klv', 'return' => 'Long' ],
    'strlen' => [ 'format' => 'k', 'return' => 'Long' ],

    // Keys
    'del' => [ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Long' ],
    'delete' => [ 'alias' => 'del' ],
    'dump' => [ 'format' => 'k', 'return' => 'Raw' ],
    'exists' => [ 'format' => 'k', 'return' => '1' ],
    'expire' => [ 'format' => 'kl', 'return' => '1' ],
    'settimeout' => [ 'alias' => 'expire' ],
    'pexpire' => [ 'format' => 'kl', 'return' => '1' ],
    'expireat' => [ 'format' => 'kl', 'return' => '1' ],
    'pexpireat' => [ 'format' => 'kl', 'return' => '1' ],
    'keys' => [ 'format' => 's', 'return' => 'Vector' ],
    'getkeys' => [ 'alias' => 'keys' ],
    'migrate' => [ 'format' => 'slkll', 'return' => 'Boolean' ],
    'move' => [ 'format' => 'kl', 'return' => '1' ],
    'persist' => [ 'format' => 'k', 'return' => '1' ],
    'randomkey' => [ 'return' => 'String' ],
    'rename' => [ 'format' => 'kk', 'return' => 'Boolean' ],
    'renamekey' => [ 'alias' => 'rename' ],
    'renamenx' => [ 'format' => 'kk', 'return' => '1' ],
    'type' => [ 'format' => 'k', 'return' => 'Type' ],
    'ttl' => [ 'format' => 'k', 'return' => 'Long' ],
    'pttl' => [ 'format' => 'k', 'return' => 'Long' ],
    'restore' => [ 'format' => 'kls', 'return' => 'Boolean' ],

    // Hashes
    'hdel' => [ 'vararg' => self::VAR_KEY_FIRST, 'return' => 'Long' ],
    'hexists' => [ 'format' => 'ks', 'return' => '1' ],
    'hget' => [ 'format' => 'ks', 'return' => 'Serialized' ],
    'hgetall' => [ 'format' => 'k', 'return' => 'Map',
                                    'retargs' => [false,true] ],
    'hincrby' => [ 'format' => 'ksl', 'return' => 'Long' ],
    'hincrbyfloat' => [ 'format' => 'ksd', 'return' => 'Double' ],
    'hkeys' => [ 'format' => 'k', 'return' => 'Vector' ],
    'hlen' => [ 'format' => 'k', 'return' => 'Long' ],
    'hset' => [ 'format' => 'ksv', 'return' => 'Long' ],
    'hsetnx' => [ 'format' => 'ksv', 'return' => '1' ],
    'hvals' => [ 'format' => 'k', 'return' => 'Vector', 'retargs' => [1] ],

    // Lists
    'blpop' => [ 'vararg' => self::VAR_KEY_ALL_AND_TIMEOUT,
                 'return' => 'Vector', 'retargs' => [1] ],
    'brpop' => [ 'vararg' => self::VAR_KEY_ALL_AND_TIMEOUT,
                 'return' => 'Vector', 'retargs' => [1] ],
    'brpoplpush' => [ 'format' => 'kkl', 'return' => 'Serialized' ],
    'lindex' => [ 'format' => 'kl', 'return' => 'Serialized' ],
    'lget' => [ 'alias' => 'lindex' ],
    'linsert' => [ 'format' => 'kpkv', 'return' => 'Long' ],
    'llen' => [ 'format' => 'k', 'return' => 'Long', 'cmd' => 'LLEN' ],
    'lsize' => [ 'alias' => 'llen' ],
    'lpop' => [ 'format' => 'k', 'return' => 'Serialized' ],
    'lpush' => [ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                 'return' => 'Long' ],
    'lpushx' => [ 'format' => 'kl', 'return' => 'Long' ],
    'lrange' => [ 'format' => 'kll', 'return' => 'Vector', 'retargs' => [1] ],
    'lgetrange' => [ 'alias' => 'lrange' ],
    'lrem' => [ 'format' => 'kvl', 'return' => 'Long' ],
    'lremove' => [ 'alias' => 'lrem' ],
    'lset' => [ 'format' => 'klv', 'return' => 'Boolean' ],
    'ltrim' => [ 'format' => 'kll', 'return' => 'Boolean' ],
    'listtrim' => [ 'alias' => 'ltrim' ],
    'rpop' => [ 'format' => 'k', 'return' => 'Serialized' ],
    'rpoplpush' => [ 'format' => 'kk', 'return' => 'Serialized' ],
    'rpush' => [ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                 'return' => 'Long' ],
    'rpushx' => [ 'format' => 'kl', 'return' => 'Long' ],

    // Sets
    'sadd' => [ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                'return' => 'Long' ],
    'scard' => [ 'format' => 'k', 'return' => 'Long' ],
    'ssize' => [ 'alias' => 'scard' ],
    'sdiff' => [ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Vector' ],
    'sdiffstore' => [ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Long' ],
    'sinter' => [ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Vector' ],
    'sinterstore' => [ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Long' ],
    'sismember' => [ 'format' => 'kv', 'return' => '1' ],
    'scontains' => [ 'alias' => 'sismember' ],
    'smembers' => [ 'format' => 'k', 'return' => 'Vector' ],
    'sgetmembers' => [ 'alias' => 'smembers' ],
    'smove' => [ 'format' => 'kkv', 'return' => '1' ],
    'spop' => [ 'format' => 'k', 'return' => 'Serialized' ],
    'srandmember' => [ 'format' => 'kl', 'return' => 'Serialized',
                       'default' => [ 1 => 1 ] ],
    'srem' => [ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                'return' => 'Long' ],
    'sremove' => [ 'alias' => 'srem' ],
    'sunion' => [ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Vector' ],
    'sunionstore' => [ 'vararg' => self::VAR_KEY_ALL, 'return' => 'Long' ],

    // zSets
    'zcard' => [ 'format' => 'k', 'return' => 'Long' ],
    'zsize' => [ 'alias' => 'zcard' ],
    'zcount' => [ 'format' => 'kss', 'return' => 'Long' ],
    'zincrby' => [ 'format' => 'kdv', 'return' => 'Double' ],
    'zinter' => [ 'alias' => 'zinterstore' ],
    'zunion' => [ 'alias' => 'zunionstore' ],
    'zrank' => [ 'format' => 'kv', 'return' => 'Long' ],
    'zrevrank' => [ 'format' => 'kv', 'return' => 'Long' ],
    'zrem' => [ 'vararg' => self::VAR_KEY_FIRST_AND_SERIALIZE,
                'return' => 'Long' ],
    'zremove' => [ 'alias' => 'zrem' ],
    'zdelete' => [ 'alias' => 'zrem' ],
    'zremrangebyrank' => [ 'format' => 'kll', 'return' => 'Long' ],
    'zdeleterangebyrank' => [ 'alias' => 'zremrangebyrank' ],
    'zremrangebyscore' => [ 'format' => 'kll', 'return' => 'Long' ],
    'zdeleterangebyscore' => [ 'alias' => 'zremrangebyscore' ],
    'zreverserange' => [ 'alias' => 'zrevrange' ],
    'zscore' => [ 'format' => 'kv', 'return' => 'Double' ],

    // Publish
    'publish' => [ 'format' => 'kv', 'return' => 'Long' ],
    /* These APIs are listed as "subject to change", avoid for now */
    'subscribe' => false,
    'psubscribe' => false,
    'unsubscribe' => false,
    'punsubscribe' => false,

    // Introspection
    '_prefix' => [ 'alias' => 'prefix' ],
    '_unserialize' => [ 'alias' => 'unserialize' ],

    // Batch Ops
    'mget' => [ 'vararg' => self::VAR_KEY_ALL,
                'return' => 'Vector', 'retargs' => [1] ],
    'getmultiple' => [ 'alias' => 'mget' ],
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
  protected $lastError = '';

  protected $timeout_connect = 0;
  protected $timeout_seconds = 0;
  protected $timeout_useconds = 0;

  protected $mode = self::ATOMIC;
  protected $multiHandler = [];
  protected $commands = [];
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
    if ($this->connection AND !feof($this->connection)) {
      // Connection seems fine
      return true;
    }

    if ((time() - $this->last_connect) < $this->retry_interval) {
      // We've tried connecting too recently, don't retry
      return false;
    }

    if ($auto_reconnect AND
        $this->doConnect($this->host, $this->port,
                         $this->timeout_connect,
                         null, $this->retry_interval,
                         $this->persistent)) {
      if ($this->password) {
        $this->auth($this->password);
      }
      if ($this->dbNumber) {
        $this->select($this->dbNumber);
      }
      return true;
    }

    // Reconnect failed, give up
    return false;
  }

  protected function sockReadLine() {
    if (!$this->checkConnection()) {
      return false;
    }
    $line = fgets($this->connection);
    if (substr($line, -2) == "\r\n") {
      $line = substr($line, 0, -2);
    }

    return $line;
  }

  protected function sockReadData(&$type) {
    $line = $this->sockReadLine();
    if (strlen($line)) {
      $type = $line[0];
      $line = substr($line, 1);
      switch ($type) {
        case self::TYPE_ERR:
          if (!strncmp($line, '-ERR SYNC ', 10)) {
            throw new RedisException("Sync with master in progress");
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
    if (($flags & self::VAR_TIMEOUT) AND
        (count($args) == 2) AND
        (is_array($args[0])) AND
        (is_int($args[1]))) {
      $args = $args[0] + [$args[1]];
    }
    if ((!($flags & self::VAR_TIMEOUT)) AND
        (count($args) == 1) AND
        (is_array($args[0]))) {
      $args = $args[0];
    }

    // Then prefix, serialie, and cast as needed
    if ($flags & self::VAR_TIMEOUT) {
      $timeout = array_pop($args);
    }
    if (($this->prefix AND ($flags & self::VAR_KEY_MASK)) OR
        ($flags & self::VAR_SERIALIZE)) {
      $first = true;
      $varkey = $flags & self::VAR_KEY_MASK;
      foreach($args as &$arg) {
        if (( $first AND ($varkey == self::VAR_KEY_FIRST)) OR
            (!$first AND ($varkey == self::VAR_KEY_NOT_FIRST)) OR
                         ($varkey == self::VAR_KEY_ALL)) {
          $arg = $this->prefix($arg);
        } else if ($flags & self::VAR_SERIALIZE) {
          $arg = $this->serialize($arg);
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
      $this->commands[] = [ 'cmd' => $cmd, 'args' => $args ];
      return true;
    }

    $clen = strlen($cmd);
    $count = count($args) + 1;
    $cmd = "*{$count}\r\n\${$clen}\r\n{$cmd}\r\n";

    while (count($args)) {
      $arg = (string)array_shift($args);
      $alen = strlen($arg);
      $cmd .= "\${$alen}\r\n{$arg}\r\n";
    }

    if (!$this->checkConnection()) {
      return false;
    }
    return (bool)fwrite($this->connection, $cmd);
  }

  protected function processCommand($cmd, /* ... */) {
    $args = func_get_args();
    array_shift($args);
    return $this->processArrayCommand($cmd, $args);
  }

  protected function serialize($str) {
    switch ($this->serializer) {
      case self::SERIALIZER_NONE:
        return $str;
      case self::SERIALIZER_PHP:
        return serialize($str);
      case self::SERIALIZER_IGBINARY:
      default:
        throw new RedisException("Not Implemented");
    }
  }

  protected function unserialize($str) {
    switch ($this->serializer) {
      case self::SERIALIZER_NONE:
        return $str;
      case self::SERIALIZER_PHP:
        return unserialize($str);
      case self::SERIALIZER_IGBINARY:
      default:
        throw new RedisException("Not Implemented");
    }
  }

  protected function processVariantResponse() {
    if ($this->mode === self::ATOMIC) {
      return $this->sockReadData($type);
    }
    $this->multiHandler[] = [ 'cb' => [$this,'processVariantResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processClientListResponse() {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = [ 'cb' => [$this,'processClientListResponse'] ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }
    $resp = $this->sockReadData($type);
    if (($type !== self::TYPE_LINE) AND
        ($type !== self::TYPE_BULK)) {
      return null;
    }
    $ret = [];
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

  protected function processSerializedResponse() {
    if ($this->mode === self::ATOMIC) {
      $resp = $this->sockReadData($type);
      return (($type === self::TYPE_LINE) OR ($type === self::TYPE_BULK))
             ? $this->unserialize($resp) : null;
    }
    $this->multiHandler[] = [ 'cb' => [$this,'processSerializedResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processBooleanResponse() {
    if ($this->mode === self::ATOMIC) {
      $resp = $this->sockReadData($type);
      return ($type === self::TYPE_LINE) AND ($resp === 'OK');
    }
    $this->multiHandler[] = [ 'cb' => [$this,'processBooleanResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processLongResponse() {
    if ($this->mode === self::ATOMIC) {
      $resp = $this->sockReadData($type);
      return ($type === self::TYPE_INT) ? ((int)$resp) : null;
    }
    $this->multiHandler[] = [ 'cb' => [$this,'processLongResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processDoubleResponse() {
    if ($this->mode === self::ATOMIC) {
      $resp = $this->sockReadData($type);
      return ($type === self::TYPE_INT) ? ((float)$resp) : null;
    }
    $this->multiHandler[] = [ 'cb' => [$this,'processDoubleResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processStringResponse() {
    if ($this->mode === self::ATOMIC) {
      $resp = $this->sockReadData($type);
      return (($type === self::TYPE_LINE) OR ($type === self::TYPE_BULK))
             ? ((string)$resp) : null;
    }
    $this->multiHandler[] = [ 'cb' => [$this,'processStringResponse'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processVectorResponse($unser = 0) {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = [ 'cb' => [$this, 'processVectorResponse'],
                                'args' => [$unser]
                           ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }

    $count = $this->sockReadData($type);
    if ($type !== self::TYPE_MULTIBULK) {
      return null;
    }

    $ret = [];
    $lineNo = 0;
    while($count--) {
      $lineNo++;
      $val = $this->sockReadData($type);
      if ($unser AND (($lineNo % $unser) == 0)) {
        $val = $this->unserialize($val);
      }
      $ret[] = $val;
    }
    return $ret;
  }

  protected function processMapResponse($unser_key, $unser_val = true) {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = [ 'cb' => [$this, 'processMapResponse'],
                                'args' => [$unser_key,$unser_val]
                              ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }

    $count = $this->sockReadData($type);
    if ($type !== self::TYPE_MULTIBULK) {
      return null;
    }

    $ret = [];
    while($count > 1) {
      $key = $this->sockReadData($type);
      if ($unser_key) {
        $key = $this->unserialize($key);
      }
      $val = $this->sockReadData($type);
      if ($unser_val) {
        $val = $this->unserialize($val);
      }
      $ret[$key] = $val;
      $count -= 2;
    }
    if ($count > 1) {
      $ret[$this->sockReadData($type)] = null;
    }
    return $ret;
  }

  protected function processAssocResponse(array $keys, $unser_val = true) {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = [ 'cb' => [$this, 'processAssocResponse'],
                                'args' => [$keys, $unser_val]
                              ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }

    $count = $this->sockReadData($type);
    if ($type !== self::TYPE_MULTIBULK) {
      return null;
    }

    $ret = [];
    while($count--) {
      $key = array_shift($keys);
      $val = $this->sockReadData($type);
      if ($unser_val) {
        $val = $this->unserialize($val);
      }
      $ret[$key] = $val;
    }
    return $ret;
  }

  protected function process1Response() {
    if ($this->mode === self::ATOMIC) {
      $resp = $this->sockReadData($type);
      return ($type === self::TYPE_INT) && ($resp === '1');
    }
    $this->multiHandler[] = [ 'cb' => [$this,'process1Response'] ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processTypeResponse() {
    if ($this->mode === self::ATOMIC) {
      $resp = $this->sockReadData($type);
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
    $this->multiHandler[] = [ 'cb' => 'processTypeResponse' ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processRawResponse() {
    if ($this->mode === self::ATOMIC) {
      return $this->sockReadLine();
    }
    $this->multiHandler[] = [ 'cb' => 'processRawResponse' ];
    if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
      return false;
    }
    return $this;
  }

  protected function processInfoResponse() {
    if ($this->mode !== self::ATOMIC) {
      $this->multiHandler[] = [ 'cb' => 'processInfoResponse' ];
      if (($this->mode === self::MULTI) && !$this->processQueuedResponse()) {
        return false;
      }
      return $this;
    }
    $resp = $this->sockReadData($type);
    if (($type !== self::TYPE_LINE) AND ($type !== self::TYPE_BULK)) {
      return false;
    }

    $ret = [];
    $lines = preg_split('/[\r\n]+/', $resp);
    foreach ($lines as $line) {
      if ((substr($line, 0, 1) == '#') OR
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
    $resp = $this->sockReadData($type);
    return ($type === self::TYPE_LINE) AND ($resp === 'QUEUED');
  }

  protected function prefix($key) {
    return $this->prefix . $key;
  }

  /**
   * Dispatches all commands in the Redis::$map list
   *
   * All other commands are handled by explicit implementations
   */
  public function __call($fname, $args) {
    $fname = strtolower($fname);
    if (!isset(self::$map[$fname])) {
      trigger_error("Call to undefined function Redis::$fname()", E_USER_ERROR);
      return null;
    }
    $func = self::$map[$fname];
    if ($func === false) {
      throw new RedisException("Redis::$fname() is currently unimplemented");
    }

    // Normalize record
    if (!empty($func['alias'])) {
      if (isset(self::$map[$func['alias']])) {
        $fname = $func['alias'];
        $func = self::$map[$fname];
      } else {
        return call_user_func_array([$this,$func['alias']],func_get_args());
      }
    }
    if (empty($func['format'])) {
      $func['format'] = isset($func['vararg']) ? '...' : '';
    }
    if (empty($func['cmd'])) {
      $func['cmd'] = strtoupper($fname);
    }
    if (empty($func['handler'])) {
      $func['handler'] = empty($func['return'])
                       ? null : "process{$func['return']}Response";
    }
    if (empty($func['retargs'])) {
      $func['retargs'] = [];
    }

    $format = $func['format'];
    $argc = count($args);

    if ($format == '...') {
      $args = $this->translateVarArgs($args, $func['vararg']);
      $this->processArrayCommand($func['cmd'], $args);
      if (empty($func['handler'])) {
        return null;
      }
      return call_user_func_array([$this, $func['handler']], $func['retargs']);
    }

    $flen = strlen($format);
    for ($i = 0; $i < $flen; $i++) {
      if (!isset($args[$i])) {
        if (isset($func['defaults']) AND
            array_key_exists($func['defaults'], $i)) {
          $args[$i] = $func['defualts'][$i];
        } else {
          trigger_error(
            "Redis::$fname requires at least $flen parameters $argc given",
            E_USER_ERROR);
          return null;
        }
      }
      switch ($format[$i]) {
        case 'k': $args[$i] = $this->prefix($args[$i]); break;
        case 'v': $args[$i] = $this->serialize($args[$i]); break;
        case 's': $args[$i] = (string)$args[$i]; break;
        case 'l': $args[$i] = (int)$args[$i]; break;
        case 'd': $args[$i] = (float)$args[$i]; break;
        case 'b': $args[$i] = (bool)$args[$i]; break;
        case 'p':
          if (($args[$i] !== self::BEFORE) AND ($args[$i] !== self::AFTER)) {
            trigger_error(
              "Argument $i to Redis::$fname must be 'before' or 'after'",
              E_USER_ERROR);
            return null;
          } break;
      }
    }
    $this->processArrayCommand($func['cmd'], $args);
    if (empty($func['handler'])) {
      return null;
    }
    return call_user_func_array([$this, $func['handler']], $func['retargs']);
  }

  /* --------------------------------------------------------------------- */

  protected function doConnect($host,
                               $port,
                               $timeout,
                               $persistent_id,
                               $retry_interval,
                               $persistent = false) {
    if (!empty($persistent_id)) {
      throw new RedisException("Named persistent connections not supported");
    }

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

    if ($persistent) {
      $conn = pfsockopen($host, $port, $errno, $errstr, $timeout);
    } else {
      $conn = fsockopen($host, $port, $errno, $errstr, $timeout);
    }
    $this->last_connect = time();
    $this->host = $host;
    $this->port = $port;
    $this->retry_interval = $retry_interval;
    $this->timeout_connect = $timeout;
    $this->persistent = $persistent;
    $this->connection = $conn;
    $this->dbNumber = 0;
    $this->commands = [];
    $this->multiHandler = [];
    $this->mode = self::ATOMIC;

    if (!$conn) {
      trigger_error(
        "Failed connecting to redis server at {$host}: {$errstr}",
        E_USER_WARNING);
      return false;
    }
    stream_set_blocking($conn, true);
    stream_set_timeout($conn, $this->timeout_seconds, $this->timeout_useconds);

    return true;
  }

  protected function sortClause(array $arr, &$using_store) {
    $using_store = false;
    if (!$arr) {
      return [];
    }

    $ret = [];
    foreach(['by','sort','store','get','alpha','limit','dir'] as $k) {
      if (isset($arr[$k])) {
        $v = $arr[$k];
      } else if (isset($arr[strtoupper($k)])) {
        $v = $arr[strtoupper($k)];
      } else {
        continue;
      }

      if (($k == 'get') AND is_array($v)) {
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
        if (is_array($val) AND (count($val) == 2)) {
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

  public function __destruct() {

  }
}
