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

/**
 * Server socket class
 */
<<Oncalls('thrift')>> // @oss-disable
final class TServerSocket {
  protected ?string $host;
  protected int $port;
  protected ?resource $handle;

  private int $sendBufferSize;
  private int $recvBufferSize;

  public function __construct(
    int $port,
    int $sendBufferSize = 512,
    int $recvBufferSize = 512,
  )[] {
    $this->host = null;
    $this->port = $port;
    $this->handle = null;
    $this->sendBufferSize = $sendBufferSize;
    $this->recvBufferSize = $recvBufferSize;
  }

  public function listen()[leak_safe]: void {
    foreach (vec['[::]', '0.0.0.0'] as $addr) {
      $errno = 0;
      $errstr = '';
      $this->handle = (
        () ==> PHP\stream_socket_server(
          'tcp://'.$addr.':'.$this->port,
          inout $errno,
          inout $errstr,
          STREAM_SERVER_BIND | STREAM_SERVER_LISTEN,
        )
      )();

      if ($this->handle !== false) {
        if ($this->port === 0) {
          $socket_name =
            PHP\stream_socket_get_name($this->handle as nonnull, false);
          $address = Str\split($socket_name, ':');
          $this->port = PHP\intval($address[C\count($address) - 1]);
        }
        break;
      }
    }
  }

  public function accept(
    int $timeout = -1,
  )[leak_safe]: ?TBufferedTransport<TSocket> {
    if ($timeout !== 0) {
      $client = $this->socketAccept($timeout);
    } else {
      $client = PHPism_FIXME::suppressAllErrors(
        ()[leak_safe] ==> $this->socketAccept($timeout),
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
    $transport = new TBufferedTransport<TSocket>(
      $socket,
      $this->sendBufferSize,
      $this->recvBufferSize,
    );

    return $transport;
  }

  private function socketAccept(int $timeout)[leak_safe]: dynamic {
    return PHP\stream_socket_accept($this->handle as nonnull, (float)$timeout);
  }

  public function close()[leak_safe]: void {
    if ($this->handle !== null) {
      HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
        ()[defaults] ==> PHP\fclose($this->handle as nonnull),
        'Blocked by a migration of builtins_file to coeffects (T107309662).',
      );
      $this->handle = null;
    }
  }

  public function port()[]: int {
    return $this->port;
  }
}
