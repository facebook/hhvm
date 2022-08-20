<?hh
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
 * @package thrift.transport
 */

/** Inherits from THttpClient */

/**
 * Multi-IP HTTP client for Thrift. The client randomly selects and uses a host
 * from its pool.
 *
 * @package thrift.transport
 */
class THttpClientPool extends THttpClient {

  /**
   * Servers belonging to this pool. Array of associative arrays with 'host' and
   * 'port' keys.
   */
  protected array<(string, int)> $servers_ = array();

  /**
   * Try hosts in order? or randomized?
   *
   * @var bool
   */
  private bool $randomize_ = true;

  /**
   * The number of times to try to connect to a particular endpoint.
   *
   * @var int
   */
  private int $numTries_ = 1;

  /**
   * Make a new multi-IP HTTP client.
   *
   * @param array  $hosts   List of remote hostnames or IPs.
   * @param mixed  $ports   Array of remote ports, or a single
   *                        common port.
   * @param string $uri     URI of the endpoint.
   * @param string $scheme  http or https
   */
  public function __construct(
    KeyedContainer<mixed, string> $hosts,
    KeyedContainer<mixed, int> $ports,
    string $uri = '',
    string $scheme = 'http',
    ?(function(string): bool) $debugHandler = null,
  ) {
    parent::__construct('', 0, $uri, $scheme, $debugHandler);

    foreach ($hosts as $key => $host) {
      $this->servers_[] = tuple($host, $ports[$key]);
    }
  }

  /**
   * Add a server to the pool. This function does not prevent you from adding a
   * duplicate server entry.
   *
   * @param string $host  hostname or IP
   * @param int    $port  port
   */
  public function addServer(string $host, int $port): void {
    $this->servers_[] = tuple($host, $port);
  }

  /**
   * Turns random endpoint selection on or off.
   *
   * @param bool $randomize
   */
  public function setRandomize(bool $randomize): void {
    $this->randomize_ = $randomize;
  }

  /**
   * Sets how many times to try to connect to an endpoint before giving up on
   * it.
   *
   * @param int $numTries
   */
  public function setNumTries(int $numTries): void {
    $this->numTries_ = $numTries;
  }

  /**
   * Trys to open a connection and send the actual HTTP request to the first
   * available server in the pool.
   */
  public function flush(): void {
    if ($this->randomize_) {
      shuffle($this->servers_);
    }

    foreach ($this->servers_ as $server) {
      $this->host_ = $server[0];
      $this->port_ = $server[1];

      $j = $this->numTries_;
      while ($j > 0) {
        try {
          parent::flush();
          return;
        } catch (TTransportException $e) {
          if ($this->debug_) {
            call_user_func($this->debugHandler_, $e->getMessage());
          }
          --$j;
        }
      }
    }

    $this->host_ = '';
    $this->port_ = 0;
    $error =
      'THttpClientPool: Could not connect to any of the servers '.
      'in the pool';
    throw new TTransportException($error, TTransportException::NOT_OPEN);
  }
}
