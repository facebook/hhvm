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
 * Base interface for a transport agent.
 *
 * @package thrift.transport
 */
<<Oncalls('thrift')>> // @oss-disable
abstract class TTransport {

  /**
   * Name of the service targeted (for logging).
   */
  protected ?string $service_ = null;

  /**
   * Whether this transport is open.
   *
   * @return boolean true if open
   */
  public abstract function isOpen()[]: bool;

  /**
   * Open the transport for reading/writing
   *
   * @throws TTransportException if cannot open
   */
  public abstract function open()[zoned_shallow]: void;

  /**
   * Close the transport.
   */
  public abstract function close()[zoned_shallow]: void;

  /**
   * Read some data into the array.
   *
   * @param int    $len How much to read
   * @return string The data that has been read
   * @throws TTransportException if cannot read any more data
   */
  public abstract function read(int $len)[zoned_shallow]: string;

  /**
   * Guarantees that the full amount of data is read.
   *
   * @return string The data, of exact length
   * @throws TTransportException if cannot read data
   */
  public function readAll(int $len)[zoned_shallow]: string {
    // return $this->read($len);

    $data = '';
    for ($got = Str\length($data); $got < $len; $got = Str\length($data)) {
      $data .= $this->read($len - $got);
    }
    return $data;
  }

  /**
   * Writes the given data out.
   *
   * @param string $buf  The data to write
   * @throws TTransportException if writing fails
   */
  public abstract function write(string $buf)[zoned_shallow]: void;

  /**
   * Flushes any pending data out of a buffer
   *
   * @throws TTransportException if a writing error occurs
   */
  public function flush()[zoned_shallow]: void {}

  /**
   * Flushes any pending data out of a buffer for a oneway call
   *
   * @throws TTransportException if a writing error occurs
   */
  public function onewayFlush()[zoned_shallow]: void {
    // Default to flush()
    $this->flush();
  }/* BEGIN_STRIP */

  /**
   * Name of the transport (e.g.: socket).
   */
  public function getTransportType()[]: string {
    return '';
  }

  /**
   * Sets the service name.
   */
  public function setService(string $service)[write_props]: void {
    $this->service_ = $service;
  }

  public function getService()[]: ?string {
    return $this->service_;
  }

  final public function incrCount(
    ProfilingCounterCount $counter,
    int $count = 1,
  )[leak_safe]: void {
    ProfilingCounters::incrCount($counter, $count);
  }

  final public function incrDuration(
    ProfilingCounterDuration $counter,
    float $duration,
  )[leak_safe]: void {
    $service = $this->getTransportType().':'.(string)$this->service_;
    ProfilingCounters::incrDuration($counter, $duration, $service);
  }
  /* END_STRIP */
}
