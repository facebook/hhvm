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
 * HTTP client for Thrift
 *
 * @package thrift.transport
 */
class THttpClient extends TTransport implements IThriftRemoteConn {

  /**
   * The host to connect to
   *
   * @var string
   */
  protected string $host_;

  /**
   * The port to connect on
   *
   * @var int
   */
  protected int $port_;

  /**
   * The URI to request
   *
   * @var string
   */
  protected string $uri_;

  /**
   * The scheme to use for the request, i.e. http, https
   *
   * @var string
   */
  protected string $scheme_;

  /**
   * Buffer for the HTTP request data
   *
   * @var string
   */
  protected string $buf_;

  /**
   * Input data stream.
   *
   * @var resource
   */
  protected string $data_;

  /**
   * Error message
   *
   * @var string
   */
  protected ?string $errstr_;

  /**
   * Read timeout
   *
   * @var float
   */
  protected ?float $timeout_;

  /**
   * Custom HTTP headers
   *
   * @var array
   */
  protected ?KeyedContainer<string, string> $custom_headers_;

  /**
   * Debugging on?
   *
   * @var bool
   */
  protected bool $debug_ = false;

  /**
   * Debug handler
   *
   * @var mixed
   */
  protected (function(string): bool) $debugHandler_;

  /**
   * Specifies the maximum number of bytes to read
   * at once from internal stream.
   */
  protected ?int $maxReadChunkSize_ = null;

  /**
   * Path to a custom CA bundle to use when verifying an HTTPS certificate.
   * Useful when talking to a service with a cert signed by the FB CA.
   */
  protected ?string $caInfo_;

  /**
   * Make a new HTTP client.
   *
   * @param string $host
   * @param int    $port
   * @param string $uri
   */
  public function __construct(
    string $host,
    int $port = 80,
    string $uri = '',
    string $scheme = 'http',
    ?(function(string): bool) $debugHandler = null,
  ) {
    if ($uri && ($uri[0] != '/')) {
      $uri = '/'.$uri;
    }
    $this->scheme_ = $scheme;
    $this->host_ = $host;
    $this->port_ = $port;
    $this->uri_ = $uri;
    $this->buf_ = '';
    $this->data_ = '';
    $this->errstr_ = '';
    $this->timeout_ = null;
    $this->custom_headers_ = null;
    $this->debugHandler_ = $debugHandler ?: fun('error_log');
  }

  /**
   * Sets the internal max read chunk size.
   * null for no limit (default).
   */
  public function setMaxReadChunkSize(int $maxReadChunkSize): void {
    $this->maxReadChunkSize_ = $maxReadChunkSize;
  }

  public function setCAInfo(string $path_to_ca_bundle): void {
    $this->caInfo_ = $path_to_ca_bundle;
  }

  /**
   * Set read timeout
   *
   * @param float $timeout
   */
  public function setTimeoutSecs(float $timeout): void {
    $this->timeout_ = $timeout;
  }

  /**
   * Set custom headers
   *
   * @param array of key value pairs of custom headers
   */
  public function setCustomHeaders(
    KeyedContainer<string, string> $custom_headers,
  ): void {
    $this->custom_headers_ = $custom_headers;
  }

  /**
   * Get custom headers
   *
   * @return array key value pairs of custom headers
   */
  public function getCustomHeaders(): ?KeyedContainer<string, string> {
    return $this->custom_headers_;
  }

  /**
   * Get the remote host that this transport is connected to.
   *
   * @return string host
   */
  public function getHost(): string {
    return $this->host_;
  }

  /**
   * Get the remote port that this transport is connected to.
   *
   * @return int port
   */
  public function getPort(): int {
    return $this->port_;
  }

  /**
   * Get the scheme used to connect to the remote server.
   *
   * @return string scheme
   */
  public function getScheme(): string {
    return $this->scheme_;
  }

  /**
   * Sets debugging output on or off
   *
   * @param bool $debug
   */
  public function setDebug(bool $debug): void {
    $this->debug_ = $debug;
  }

  /**
   * Whether this transport is open.
   *
   * @return boolean true if open
   */
  public function isOpen(): bool {
    return true;
  }

  /**
   * Open the transport for reading/writing
   *
   * @throws TTransportException if cannot open
   */
  public function open(): void {}

  /**
   * Close the transport.
   */
  public function close(): void {
    $this->data_ = '';
  }

  /**
   * Read some data into the array.
   *
   * @param int    $len How much to read
   * @return string The data that has been read
   * @throws TTransportException if cannot read any more data
   */
  public function read(int $len): string {
    if ($this->maxReadChunkSize_ !== null) {
      $len = min($len, $this->maxReadChunkSize_);
    }

    if (strlen($this->data_) < $len) {
      $data = $this->data_;
      $this->data_ = '';
    } else {
      $data = substr($this->data_, 0, $len);
      $this->data_ = substr($this->data_, $len);
    }

    if ($data === false || $data === '') {
      throw new TTransportException(
        'THttpClient: Could not read '.
        $len.
        ' bytes from '.
        $this->host_.
        ':'.
        $this->port_.
        '/'.
        $this->uri_.
        " Reason: ".
        $this->errstr_,
        TTransportException::UNKNOWN,
      );
    }
    return $data;
  }

  /**
   * Writes some data into the pending buffer
   *
   * @param string $buf  The data to write
   * @throws TTransportException if writing fails
   */
  public function write(string $buf): void {
    $this->buf_ .= $buf;
  }

  /**
   * Opens and sends the actual request over the HTTP connection
   *
   * @throws TTransportException if a writing error occurs
   */
  public function flush(): void {
    $host = $this->host_.':'.$this->port_;
    $user_agent = 'PHP/THttpClient';
    /* HH_FIXME[2050] We know $_SERVER exists, even if Hack doesn't */
    $script = (string) urlencode(basename($_SERVER['SCRIPT_FILENAME']));
    if ($script) {
      $user_agent .= ' ('.$script.')';
    }

    $curl = curl_init($this->scheme_.'://'.$host.$this->uri_);

    $headers = array(
      'Host: '.$host,
      'Accept: application/x-thrift',
      'User-Agent: '.$user_agent,
      'Content-Type: application/x-thrift',
      'Content-Length: '.(string) strlen($this->buf_),
    );

    if ($this->custom_headers_ !== null) {
      foreach ($this->custom_headers_ as $header => $value) {
        $headers[] = $header.': '.$value;
      }
    }
    curl_setopt($curl, CURLOPT_PROXY, '');
    curl_setopt($curl, CURLOPT_HTTPHEADER, $headers);
    curl_setopt($curl, CURLOPT_POST, true);
    curl_setopt($curl, CURLOPT_MAXREDIRS, 1);
    curl_setopt($curl, CURLOPT_FOLLOWLOCATION, true);
    curl_setopt($curl, CURLOPT_POSTFIELDS, $this->buf_);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($curl, CURLOPT_BINARYTRANSFER, true);
    curl_setopt($curl, CURLOPT_TIMEOUT, $this->timeout_);
    if ($this->caInfo_ !== null) {
      curl_setopt($curl, CURLOPT_CAINFO, $this->caInfo_);
    }

    $this->buf_ = '';
    $this->data_ = curl_exec($curl);

    $this->errstr_ = curl_error($curl);
    curl_close($curl);

    // Connect failed?
    if ($this->data_ === false) {
      $error = 'THttpClient: Could not connect to '.$host.$this->uri_;
      throw new TTransportException($error, TTransportException::NOT_OPEN);
    }
  }

  public function isReadable(): bool {
    return (bool) $this->data_;
  }

  public function isWritable(): bool {
    return true;
  }

  public function getRecvTimeout(): int {
    return 0;
  }
  public function setRecvTimeout(int $timeout): void { /* do nothing */
  }
}
