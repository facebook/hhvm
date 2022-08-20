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

/**
 * Server socket class
 */
class TServerSocket {
  protected ?string $host;
  protected int $port;
  protected ?resource $handle;

  private int $send_buffer_size;
  private int $recv_buffer_size;
  public function __construct(
    int $port,
    int $send_buffer_size = 512,
    int $recv_buffer_size = 512,
  ) {
    $this->host = null;
    $this->port = $port;
    $this->handle = null;
    $this->send_buffer_size = $send_buffer_size;
    $this->recv_buffer_size = $recv_buffer_size;
  }

  public function listen(): void {
    foreach (varray['[::]', '0.0.0.0'] as $addr) {
      $errno = 0;
      $errstr = '';
      $this->handle = PHP\stream_socket_server(
        'tcp://'.$addr.':'.$this->port,
        &$errno,
        &$errstr,
        STREAM_SERVER_BIND | STREAM_SERVER_LISTEN,
      );

      if ($this->handle !== false) {
        if ($this->port === 0) {
          $address = Str\split(
            PHP\stream_socket_get_name($this->handle, false),
            ':',
          );
          $this->port = PHP\intval($address[PHP\count($address) - 1]);
        }
        break;
      }
    }
  }

  public function accept(int $timeout = -1): ?TBufferedTransport {
    if ($timeout !== 0) {
      $client = PHP\stream_socket_accept(
        nullthrows($this->handle),
        (float)$timeout,
      );
    } else {
      $client = @PHP\stream_socket_accept(
        nullthrows($this->handle),
        (float)$timeout,
      );
    }

    if (!$client) {
      return null;
    }

    $socket = new TSocket();
    $socket->setHandle($client);

    // We need to create a buffered transport because TSocket's isReadable
    // is not reliable in PHP (hphp is fine) and buffered transport is more
    // efficient (60% faster in my tests)
    $transport = new TBufferedTransport(
      $socket,
      $this->send_buffer_size,
      $this->recv_buffer_size,
    );

    return $transport;
  }

  public function close(): void {
    /* HH_FIXME[4016] Exposed by banning unset and isset in partial mode */
    if (isset($this->handle)) {
      PHP\fclose($this->handle);
      $this->handle = null;
    }
  }

  public function port(): int {
    return $this->port;
  }
}
