<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

/** Inherits from Socket */

/**
 * Sockets implementation of the TTransport interface that allows connection
 * to a pool of servers.
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
final class TSocketPool extends TSocket {
  /**
   * Remote servers. List of host:port pairs.
   */
  private vec<(string, int)> $servers_ = vec[];

  /**
   * How many times to retry each host in connect
   *
   * @var int
   */
  private int $numRetries_ = 1;

  /**
   * Retry interval in seconds, how long to not try a host if it has been
   * marked as down.
   *
   * @var int
   */
  private int $retryInterval_ = 60;

  /**
   * Max consecutive failures before marking a host down.
   *
   * @var int
   */
  private int $maxConsecutiveFailures_ = 1;

  /**
   * Try hosts in order? or Randomized?
   *
   * @var bool
   */
  private bool $randomize_ = true;

  /**
   * Always try last host, even if marked down?
   *
   * @var bool
   */
  private bool $alwaysTryLast_ = true;

  /**
   * Always retry the host without wait if there was a transient
   * connection failure (such as Resource temporarily unavailable).
   * If this is set, the wait_and_retry mechanism will ignore the
   * value in $retryInterval_ for transient failures.
   *
   * @var bool
   */
  private bool $alwaysRetryForTransientFailure_ = false;

  const int ERROR_RESOURCE_TEMPORARILY_UNAVAILABLE = 11;

  /**
   * Socket pool constructor
   *
   * @param array  $hosts        List of remote hostnames
   * @param mixed  $ports        Array of remote ports, or a single common port
   * @param bool   $persist      Whether to use a persistent socket
   */
  public function __construct(
    KeyedContainer<int, string> $hosts = vec['localhost'],
    KeyedContainer<int, int> $ports = vec[9090],
    bool $persist = false,
  )[leak_safe] {
    parent::__construct('', 0, $persist);

    foreach ($hosts as $key => $host) {
      $this->servers_[] = tuple((string)$host, (int)$ports[$key]);
    }
  }

  /**
   * Add a server to the pool
   *
   * This function does not prevent you from adding a duplicate server entry.
   *
   * @param string $host hostname or IP
   * @param int $port port
   */
  public function addServer(string $host, int $port)[write_props]: void {
    $this->servers_[] = tuple($host, $port);
  }

  /**
   * Sets how many time to keep retrying a host in the connect function.
   *
   * @param int $numRetries
   */
  public function setNumRetries(int $numRetries)[write_props]: void {
    $this->numRetries_ = $numRetries;
  }

  /**
   * Sets how long to wait until retrying a host if it was marked down
   *
   * @param int $numRetries
   */
  public function setRetryInterval(int $retryInterval)[write_props]: void {
    $this->retryInterval_ = $retryInterval;
  }

  /**
   * Sets how many time to keep retrying a host before marking it as down.
   *
   * @param int $numRetries
   */
  public function setMaxConsecutiveFailures(
    int $maxConsecutiveFailures,
  )[write_props]: void {
    $this->maxConsecutiveFailures_ = $maxConsecutiveFailures;
  }

  /**
   * Turns randomization in connect order on or off.
   *
   * @param bool $randomize
   */
  public function setRandomize(bool $randomize)[write_props]: void {
    $this->randomize_ = $randomize;
  }

  /**
   * Whether to always try the last server.
   *
   * @param bool $alwaysTryLast
   */
  public function setAlwaysTryLast(bool $alwaysTryLast)[write_props]: void {
    $this->alwaysTryLast_ = $alwaysTryLast;
  }

  /**
   * Whether to always retry the host without wait for
   * transient connection failures
   *
   * @param bool $alwaysRetry
   */
  <<__Deprecated(
    'This function was found unused by CodemodRuleDeprecateUnusedClassMethod',
  )>>
  public function setAlwaysRetryForTransientFailure(
    bool $alwaysRetry,
  )[write_props]: void {
    $this->alwaysRetryForTransientFailure_ = $alwaysRetry;
  }

  /**
   * Connects the socket by iterating through all the servers in the pool
   * and trying to find one that:
   * 1. is not marked down after consecutive failures
   * 2. can really be connected to
   *
   * @return bool  false: any IP in the pool failed to connect before returning
   *               true: no failures
   */
  <<__Override>>
  public function open()[leak_safe]: void {
    // Check if we want order randomization
    if ($this->randomize_) {
      // warning: don't use shuffle here because it leads to uneven
      // load distribution
      $n = C\count($this->servers_);
      $s = $this->servers_;
      for ($i = 1; $i < $n; $i++) {
        $j = PHP\mt_rand(0, $i);
        $tmp = $s[$i];
        $s[$i] = $s[$j];
        $s[$j] = $tmp;
      }
      $this->servers_ = $s;
    }

    // Count servers to identify the "last" one
    $numServers = C\count($this->servers_);

    $fail_reason = dict[]; // reasons of conn failures
    for ($i = 0; $i < $numServers; ++$i) {

      // host port is stored as an array
      list($host, $port) = $this->servers_[$i];

      $failtimeKey = TSocketPool::getAPCFailtimeKey($host, $port);
      // Cache miss? Assume it's OK
      $lastFailtime = self::apcFetch($failtimeKey);

      $retryIntervalPassed = false;

      // Cache hit...make sure enough the retry interval has elapsed
      if ($lastFailtime > 0) {
        $elapsed = PHP\time() - $lastFailtime;
        if ($elapsed > $this->retryInterval_) {
          $retryIntervalPassed = true;
        }
      }

      // Only connect if not in the middle of a fail interval, OR if this
      // is the LAST server we are trying, just hammer away on it
      $isLastServer = false;
      if ($this->alwaysTryLast_) {
        $isLastServer = ($i === ($numServers - 1));
      }

      if (
        ($lastFailtime === 0) ||
        ($isLastServer) ||
        ($lastFailtime > 0 && $retryIntervalPassed)
      ) {

        // Set underlying TSocket params to this one/* BEGIN_STRIP */
        // fsockopen requires IPv6 addresses be bracet enclosed
        if (ip_is_valid($host)) {
          $this->host_ = IPAddress($host)->forURL();
        } else {
          // probably a hostname
          /* END_STRIP */
          $this->host_ = $host;/* BEGIN_STRIP */
        }
        /* END_STRIP */
        $this->port_ = $port;

        // Try up to numRetries_ connections per server
        for ($attempt = 0; $attempt < $this->numRetries_; $attempt++) {
          try {
            // Use the underlying TSocket open function
            parent::open();

            // Only clear the failure counts if required to do so
            if ($lastFailtime > 0) {
              self::apcStore($failtimeKey, 0);
            }

            // Successful connection, return now
            return;

          } catch (TException $_tx) {
            // Connection failed
            // keep the reason for the last try
            $errstr = $this->getErrStr();
            $errno = $this->getErrNo();
            if ($errstr !== null || $errno !== null) {
              $fail_reason[$i] = '('.(string)$errstr.'['.(string)$errno.'])';
            } else {
              $fail_reason[$i] = '(?)';
            }
          }
        }

        // For transient errors (like Resource temporarily unavailable),
        // we might want not to cache the failure.
        if (
          $this->alwaysRetryForTransientFailure_ &&
          $this->isTransientConnectFailure($this->getErrNo())
        ) {
          continue;
        }

        $_has_conn_errors = $this->recordFailure(
          $host,
          $port,
          $this->maxConsecutiveFailures_,
          $this->retryInterval_,
        );
      } else {
        $fail_reason[$i] = '(cached-down)';
      }
    }

    // Holy shit we failed them all. The system is totally ill!
    $error = 'TSocketPool: All hosts in pool are down. ';
    $hosts = vec[];
    foreach ($this->servers_ as $i => $server) {
      // array(host, port) (reasons, if exist)
      list($host, $port) = $server;/* BEGIN_STRIP */
      if (ip_is_valid($host)) {
        $host = IPAddress($host)->forURL();
      }
      /* END_STRIP */
      $h = $host.':'.$port;
      if (C\contains_key($fail_reason, $i)) {
        $h .= (string)$fail_reason[$i];
      }
      $hosts[] = $h;
    }
    $hostlist = Str\join($hosts, ',');
    $error .= '('.$hostlist.')';
    throw new TTransportException($error);
  }

  public static function getAPCFailtimeKey(string $host, int $port)[]: string {
    // Check APC cache for a record of this server being down
    return 'thrift_failtime:'.$host.':'.$port.'~';
  }

  /**
   * Record the failure in APC
   * @param string  host  dest IP
   * @param int     port  dest port
   * @param int     max_failures   max consec errors before mark host/port down
   * @param int     down_period    how long to mark the host/port down
   *
   * @return bool  if mark a host/port down
   */
  public function recordFailure(
    string $host,
    int $port,
    int $max_failures,
    int $down_period,
  )[leak_safe]: bool {
    $marked_down = false;
    // Mark failure of this host in the cache
    $consecfailsKey = 'thrift_consecfails:'.$host.':'.$port.'~';

    // Ignore APC misses (treat as 0)
    $consecfails = self::apcFetch($consecfailsKey) + 1;

    // Log and cache this failure
    if ($consecfails >= $max_failures) {
      // Store the failure time
      $failtimeKey = self::getAPCFailtimeKey($host, $port);
      $curr_time = PHP\time();
      self::apcStore($failtimeKey, $curr_time);
      $marked_down = true;

      // Clear the count of consecutive failures
      self::apcStore($consecfailsKey, 0);
    } else {
      self::apcStore($consecfailsKey, $consecfails);
    }

    return $marked_down;
  }

  /**
   * Whether a connection failure is a transient failure
   * based on the error-code
   *
   * @param int $error_code
   * @return bool true or false
   */
  private function isTransientConnectFailure(?int $error_code)[]: bool {
    if ($error_code === null) {
      return false;
    }
    // todo: add more to this list
    switch ($error_code) {
      case self::ERROR_RESOURCE_TEMPORARILY_UNAVAILABLE:
        return true;
      default:
        return false;
    }
  }

  private static function apcStore(string $key, int $value)[leak_safe]: void {
    HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
      ()[defaults] ==> PHP\apc_store($key, $value),
      'Used for stats collection. Blocked by a migration of APC to coeffects '.
      '(T107224583).',
    );
  }

  private static function apcFetch(string $key)[leak_safe]: int {
    return HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
      ()[defaults] ==> (int)PHP\fb\apc_fetch_no_success_check($key),
      'Used for stats collection. Blocked by a migration of APC to coeffects '.
      '(T107224583).',
    );
  }
}
