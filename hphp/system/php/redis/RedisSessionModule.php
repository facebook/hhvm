<?php

class RedisSessionModule implements SessionHandlerInterface {
  /**
   * Total of all connection weights, used as divisor in key distribution
   */
  protected $weight = 0;

  /**
   * Map of redis backend servers to use
   *
   * Comes from session.save_path which is a comma delimted list of URIs
   * in the form of:
   *
   *   proto://hostOrPath:optionalPort?key=val&key=val...
   *
   * Where proto is one of: tcp | unix
   *
   * When proto is 'tcp' and no port is given, Redis::DEFAULT_PORT is assumed
   *
   * Key/Value pairs are as follows:
   *  weight(int) - Likelihood of using this backend will be $weight/$total
   *  timeout(int) - Connection timeout in seconds
   *  persistent(int) - 0 or 1 for non-persistent or persistent connections
   *  prefix(string) - String prefix to apply to all session keys in the DB
   *  auth(string) - Password used to authenticate to the DB
   *  database(int) - Database shard to use
   *
   * Defaults can be found in the code below
   */
  protected $paths = [];

  public function open($save_path, $session_name) {
    $this->weight = 0;
    $this->paths  = [];

    $paths = preg_split('/[\s,]+/', $save_path, -1, PREG_SPLIT_NO_EMPTY);
    foreach ($paths as $path) {
      if (!strncmp($path, "unix:", 5)) {
        $path = 'file:' . substr($path, 5);
      }
      $url = parse_url($path);
      $args = [
        'weight' => 1,
        'timeout' => 86400,
        'persistent' => 0,
        'prefix' => 'PHPREDIS_SESSION:',
        'auth' => '',
        'database' => 0
      ];
      if (isset($url['query'])) {
        parse_str($url['query'], $query);
        foreach ($args as $key => &$val) {
          if (!isset($query[$key])) continue;
          if (is_string($val)) {
            $val = $query[$key];
          } else {
            $val = (int)$query[$key];
          }
        }
      }

      if ($args['weight'] === 0) {
        // Disabled target, skip
        continue;
      }
      if ($args['weight'] < 1) {
        error_log("Invalid weight specified for session.save_path(redis), ".
                  "assuming 1", E_USER_WARNING);
        $args['weight'] = 1;
      }

      if ($url['scheme'] == 'file') {
        $args['host'] = "unix://{$url['path']}";
        $args['port'] = null;
      } else {
        $args['host'] = $url['host'];
        $args['port'] = !empty($url['port'])
                      ? $url['port'] : Redis::DEFAULT_PORT;
      }

      $args['connection'] = null;

      $this->weight += $args['weight'];
      $this->paths[] = $args;
    }

    return true;
  }

  protected function &selectWeight($key) {
    if (count($this->paths) === 1) {
      return $this->paths[0];
    }
    $pos  = abs(unpack("I", $key));
    $pos %= $this->weight;
    foreach ($this->paths as $path) {
      $pos -= $path['weight'];
      if ($pos <= 0) {
        return $path;
      }
    }

    throw new RedisException("Ran out of weights selecting redis host ".
                             "for session: $key");
  }

  protected function connect($key) {
    $r =& $this->selectWeight($key);
    if (!empty($r['connection'])) {
      return $r['connection'];
    }

    $redis = new Redis;
    $func = ($r['persistent']) ? 'pconnect' : 'connect';
    if (!$redis->{$func}($r['host'], $r['port'], $r['timeout'])) {
      return false;
    }
    if (($r['auth'] !== '') &&
        !$redis->auth($r['auth'])) {
      return false;
    }
    if (($r['database'] !== 0) &&
        !$redis->select($r['database'])) {
      return false;
    }
    if (!$redis->setOption(Redis::OPT_PREFIX, $r['prefix'])) {
      return false;
    }

    $r['connection'] = $redis;
    return $redis;
  }

  public function close() {
    foreach ($this->paths as &$path) {
      $path['connection'] = null;
    }

    return true;
  }

  public function read($key) {
    $redis = $this->connect($key);
    return (string)$redis->get($key);
  }

  public function write($key, $value) {
    $redis = $this->connect($key);
    return $redis->set($key, $value, ini_get('session.gc_maxlifetime'));
  }

  public function destroy($key) {
    $redis = $this->connect($key);
    return $redis->del($key);
  }

  public function gc($maxlifetime) {
    // Handled by $redis->set(..., ini_get('session.gc_maxlifetime'))
    return 0;
  }
}
