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
 * @package thrift
 */

<<__ConsistentConstruct>>
abstract class ThriftClientBase implements IThriftClient {
  protected TProtocol $input_;
  protected TProtocol $output_;
  protected ?IThriftAsyncChannel $channel_;
  protected TClientAsyncHandler $asyncHandler_;
  protected TClientEventHandler $eventHandler_;

  protected int $seqid_ = 0;

  final public static function factory(
  ): (string, (function (TProtocol, ?TProtocol, ?IThriftAsyncChannel): this)) {
    return tuple(
      get_called_class(),
      function(
        TProtocol $input,
        ?TProtocol $output,
        ?IThriftAsyncChannel $channel,
      ) {
        return new static($input, $output, $channel);
      },
    );
  }

  public function __construct(
    TProtocol $input,
    ?TProtocol $output = null,
    ?IThriftAsyncChannel $channel = null,
  ) {
    $this->input_ = $input;
    $this->output_ = $output ?: $input;
    $this->channel_ = $channel;
    $this->asyncHandler_ = new TClientAsyncHandler();
    $this->eventHandler_ = new TClientEventHandler();
  }

  public function setAsyncHandler(TClientAsyncHandler $async_handler): this {
    $this->asyncHandler_ = $async_handler;
    return $this;
  }

  public function getAsyncHandler(): TClientAsyncHandler {
    return $this->asyncHandler_;
  }

  public function setEventHandler(TClientEventHandler $event_handler): this {
    $this->eventHandler_ = $event_handler;
    return $this;
  }

  public function getEventHandler(): TClientEventHandler {
    return $this->eventHandler_;
  }

  protected function getNextSequenceID(): int {
    $currentseqid = $this->seqid_;
    if ($this->seqid_ >= 0x7fffffff) {
      $this->seqid_ = 0;
    } else {
      $this->seqid_++;
    }
    return $currentseqid;
  }
}
