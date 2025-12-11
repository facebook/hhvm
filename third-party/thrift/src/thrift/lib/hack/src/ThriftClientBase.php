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

type ThriftClientFactory<T> = (classname<T>, (function(
  TProtocol,
  ?TProtocol,
  ?IThriftMigrationAsyncChannel,
): T));

<<__ConsistentConstruct, Oncalls('thrift')>> // @oss-disable
// @oss-enable <<__ConsistentConstruct>>
abstract class ThriftClientBase implements IThriftClient {
  protected TProtocol $input_;
  protected TProtocol $output_;
  protected ?IThriftMigrationAsyncChannel $channel_;
  protected TClientAsyncHandler $asyncHandler_;
  protected TClientEventHandler $eventHandler_;
  protected ?RpcOptions $options_;
  protected ?ThriftClientConfig $config_;

  protected int $seqid_ = 0;
  abstract const string THRIFT_SVC_NAME;

  public static function factory()[]: ThriftClientFactory<this> {
    return tuple(
      static::class,
      function(
        TProtocol $input,
        ?TProtocol $output,
        ?IThriftMigrationAsyncChannel $channel,
      )[defaults] {
        return new static($input, $output, $channel);
      },
    );
  }

  public function __construct(
    TProtocol $input,
    ?TProtocol $output = null,
    ?IThriftMigrationAsyncChannel $channel = null,
  )[leak_safe] {
    $this->input_ = $input;
    $this->output_ = $output ?: $input;
    $this->channel_ = $channel;
    $this->asyncHandler_ = new TClientAsyncHandler();
    $this->eventHandler_ = new TClientEventHandler();
  }

  public function setAsyncHandler(
    TClientAsyncHandler $async_handler,
  )[write_props]: this {
    $this->asyncHandler_ = $async_handler;
    return $this;
  }

  public function getAsyncHandler()[]: TClientAsyncHandler {
    return $this->asyncHandler_;
  }

  public function setEventHandler(
    TClientEventHandler $event_handler,
  )[write_props]: this {
    $this->eventHandler_ = $event_handler;
    return $this;
  }

  public function getEventHandler()[]: TClientEventHandler {
    return $this->eventHandler_;
  }

  final public function getHHFrameMetadata()[]: ?string {
    return null;
  }

  protected function getNextSequenceID()[write_props]: int {
    $currentseqid = $this->seqid_;
    if ($this->seqid_ >= 0x7fffffff) {
      $this->seqid_ = 0;
    } else {
      $this->seqid_++;
    }
    return $currentseqid;
  }

  protected static function defaultOptions()[read_globals]: RpcOptions {
    return new RpcOptions();
  }

  /**
   * For use when you want to set specific options on THE NEXT RPC invocation
   * utilizing this client.
   *
   * You should call this IMMEDIATELY prior to your rpc call, as in
   * await $client->onceWithOptions($options)->myApiInvocation();
   *
   * IMPORTANT: as part of the next RPC call, these options will be reset, so
   * if you want to apply these options to multiple RPCs calls, you will need
   * to utilize this function repeatedly
   */
  public function onceWithOptions(RpcOptions $rpc_options)[write_props]: this {
    invariant(
      $this->options_ is null,
      'trying to set options twice on the same client at the same time',
    );
    $this->options_ = $rpc_options;
    return $this;
  }

  public function setThriftClientConfig(
    ThriftClientConfig $config,
  )[write_props]: this {
    $this->config_ = $config;
    return $this;
  }

  protected function getAndResetOptions()[write_props]: ?RpcOptions {
    $options = $this->options_;
    $this->options_ = null;
    return $options;
  }

  protected function sendImplHelper(
    IThriftStruct $args,
    string $function_name,
    bool $is_one_way,
    string $service_name,
  ): int {
    $currentseqid = $this->getNextSequenceID();
    try {
      $this->eventHandler_
        ->preSend($function_name, $args, $currentseqid, $service_name);
      if ($this->output_ is \TBinaryProtocolAccelerated) {
        thrift_protocol_write_binary(
          $this->output_,
          $function_name,
          TMessageType::CALL,
          $args,
          $currentseqid,
          $this->output_->isStrictWrite(),
          $is_one_way,
        );
      } else if ($this->output_ is \TCompactProtocolAccelerated) {
        thrift_protocol_write_compact2(
          $this->output_,
          $function_name,
          TMessageType::CALL,
          $args,
          $currentseqid,
          $is_one_way,
          TCompactProtocolBase::VERSION,
        );
      } else {
        $this->output_->writeMessageBegin(
          $function_name,
          TMessageType::CALL,
          $currentseqid,
        );
        $args->write($this->output_);
        $this->output_->writeMessageEnd();
        if ($is_one_way) {
          $this->output_->getTransport()->onewayFlush();
        } else {
          $this->output_->getTransport()->flush();
        }
      }
    } catch (\THandlerShortCircuitException $ex) {
      switch ($ex->resultType) {
        case THandlerShortCircuitException::R_EXPECTED_EX:
        case THandlerShortCircuitException::R_UNEXPECTED_EX:
          $this->eventHandler_
            ->sendError($function_name, $args, $currentseqid, $ex->result);
          throw $ex->result;
        case THandlerShortCircuitException::R_SUCCESS:
        default:
          $this->eventHandler_->postSend($function_name, $args, $currentseqid);
          return $currentseqid;
      }
    } catch (\Exception $ex) {
      $this->eventHandler_
        ->sendError($function_name, $args, $currentseqid, $ex);
      throw $ex;
    }
    $this->eventHandler_->postSend($function_name, $args, $currentseqid);
    return $currentseqid;
  }

  protected function recvImplHelper<
    TResulttype as IResultThriftStruct with { type TResult = TRet },
    TRet,
  >(
    classname<TResulttype> $result,
    string $name,
    bool $is_return_void,
    ?int $expectedsequenceid,
    shape(?'read_options' => int) $options = shape(),
  ): TRet {
    try {
      $this->eventHandler_->preRecv($name, $expectedsequenceid);
      if ($this->input_ is \TBinaryProtocolAccelerated) {
        $result = thrift_protocol_read_binary(
          $this->input_,
          HH\class_to_classname($result),
          $this->input_->isStrictRead(),
          Shapes::idx($options, 'read_options', 0),
        );
      } else if ($this->input_ is \TCompactProtocolAccelerated) {
        $result = thrift_protocol_read_compact(
          $this->input_,
          HH\class_to_classname($result),
          Shapes::idx($options, 'read_options', 0),
        );
      } else {
        $rseqid = 0;
        $fname = '';
        $mtype = 0;

        $this->input_
          ->readMessageBegin(inout $fname, inout $mtype, inout $rseqid);
        if ($mtype === TMessageType::EXCEPTION) {
          $x = new \TApplicationException();
          $x->read($this->input_);
          $this->input_->readMessageEnd();
          throw $x;
        }
        $result = $result::withDefaultValues();
        $result->read($this->input_);
        $this->input_->readMessageEnd();
        if ($expectedsequenceid !== null && ($rseqid !== $expectedsequenceid)) {
          throw new \TProtocolException(
            $name." failed: sequence id is out of order",
          );
        }
      }
    } catch (THandlerShortCircuitException $ex) {
      switch ($ex->resultType) {
        case THandlerShortCircuitException::R_EXPECTED_EX:
          $this->eventHandler_
            ->recvException($name, $expectedsequenceid, $ex->result);
          throw $ex->result;
        case THandlerShortCircuitException::R_UNEXPECTED_EX:
          $this->eventHandler_
            ->recvError($name, $expectedsequenceid, $ex->result);
          throw $ex->result;
        case THandlerShortCircuitException::R_SUCCESS:
        default:
          $this->eventHandler_
            ->postRecv($name, $expectedsequenceid, $ex->result);
          // this should just always be null in the ThriftSyncStructWithoutResult case
          return $ex->result;
      }
    } catch (\Exception $ex) {
      $this->eventHandler_->recvError($name, $expectedsequenceid, $ex);
      throw $ex;
    }

    if (!$is_return_void) {
      if ($result is ThriftSyncStructWithResult) {
        $successful_result = $result->success;
      } else if ($result is ThriftAsyncStructWithResult) {
        $successful_result = $result->success;
      } else {
        $successful_result = null;
      }
      if ($successful_result is nonnull) {
        $this->eventHandler_
          ->postRecv($name, $expectedsequenceid, $successful_result);
        return HH\FIXME\UNSAFE_CAST<mixed, TRet>(
          $successful_result,
          'FIXME[4110] Type error uncovered by True Types, potential incompleteness (see https://fburl.com/workplace/ngmpvd6l)',
        );
      }
    }

    $exception = $result->checkForException();
    if ($exception is nonnull) {
      if ($exception is rfe_ScubaUserError) {
        $exception->message = TAAL::blameThroughDirectories(
          $exception->message,
          vec[
            'flib/__generated__/ThriftMeerkatStep/fbcode/rfe/',
            'flib/thrift/core/',
            'flib/intern/site/web/webfoundation/',
          ],
        );
      }
      $this->eventHandler_
        ->recvException($name, $expectedsequenceid, $exception);
      throw $exception;
    }

    if ($is_return_void) {
      $this->eventHandler_
        ->postRecv($name, $expectedsequenceid, null);
      return
        HH\FIXME\UNSAFE_CAST<null, TRet>(null, 'FIXME[4110] TRet is void here');
    } else {
      $x = new \TApplicationException(
        $name." failed: unknown result",
        TApplicationException::MISSING_RESULT,
      );
      $this->eventHandler_
        ->recvError($name, $expectedsequenceid, $x);
      throw $x;
    }
  }

  protected async function genAwaitResponse<
    TResulttype as IResultThriftStruct with { type TResult = TRet },
    TRet,
  >(
    classname<TResulttype> $result,
    string $name,
    bool $is_return_void,
    int $expectedsequenceid,
    RpcOptions $rpc_options,
    shape(?'read_options' => int) $options = shape(),
  ): Awaitable<(TRet, ?dict<string, string>)> {
    $channel = $this->channel_;
    $out_transport = $this->output_->getTransport();
    $in_transport = $this->input_->getTransport();
    $read_headers = null;
    if (
      $channel !== null &&
      $out_transport is \TMemoryBuffer &&
      $in_transport is \TMemoryBuffer
    ) {
      $msg = $out_transport->getBuffer();
      $out_transport->resetBuffer();
      list($result_msg, $read_headers) =
        await $channel->genSendRequestResponse($rpc_options, $msg);
      $in_transport->resetBuffer();
      $in_transport->write($result_msg);
    } else {
      await $this->asyncHandler_->genWait($expectedsequenceid);
    }
    $response = $this->recvImplHelper(
      $result,
      $name,
      $is_return_void,
      $expectedsequenceid,
      $options,
    );
    await $this->asyncHandler_->genAfter<TRet>($name, $response);
    return tuple($response, $read_headers);
  }

  protected async function genAwaitNoResponse(
    RpcOptions $rpc_options,
  ): Awaitable<void> {
    $channel = $this->channel_;
    $out_transport = $this->output_->getTransport();
    if ($channel !== null && $out_transport is \TMemoryBuffer) {
      $msg = $out_transport->getBuffer();
      $out_transport->resetBuffer();
      await $channel->genSendRequestNoResponse($rpc_options, $msg);
    }
  }

  protected async function genAwaitStreamResponse<
    TFirstResponseType as IResultThriftStruct with {
      type TResult = TFirstType },
    TFirstType,
    TStreamResponseType as IResultThriftStruct with {
      type TResult = TStreamType },
    TStreamType,
  >(
    classname<TFirstResponseType> $first_response_type,
    class<TStreamResponseType> $stream_response_type,
    string $name,
    bool $is_first_response_null,
    int $expectedsequenceid,
    RpcOptions $rpc_options,
    shape(?'read_options' => int) $options = shape(),
  ): Awaitable<\ResponseAndStream<TFirstType, TStreamType>> {

    $channel = $this->channel_;
    $out_transport = $this->output_->getTransport();
    $in_transport = $this->input_->getTransport();
    invariant(
      $channel !== null &&
        $out_transport is \TMemoryBuffer &&
        $in_transport is \TMemoryBuffer,
      "Stream methods require nonnull channel and TMemoryBuffer transport",
    );
    $msg = $out_transport->getBuffer();
    $out_transport->resetBuffer();
    list($result_msg, $_read_headers, $stream) =
      await $channel->genSendRequestStreamResponse($rpc_options, $msg);
    $disable16kblimit = $this->config_?->getStreamDisable16KBLimit() ?? false;
    if ($disable16kblimit) {
      $stream->disable16KBBufferingPolicy();
    }

    $stream_gen = $stream->gen<TStreamType>(
      ThriftStreamingSerializationHelpers::decodeStreamHelper(
        $stream_response_type,
        $name,
        $this->input_,
        $options,
      ),
    );
    $in_transport->resetBuffer();
    $in_transport->write($result_msg);
    $first_response = $this->recvImplHelper(
      $first_response_type,
      $name,
      $is_first_response_null,
      $expectedsequenceid,
      $options,
    );
    await $this->asyncHandler_
      ->genAfter<TFirstType>($name, $first_response);
    return new \ResponseAndStream<TFirstType, TStreamType>(
      $first_response,
      $stream_gen,
    );
  }

  protected async function genAwaitSinkResponse<
    TSinkFirstResponseType as IResultThriftStruct with {
      type TResult = TSinkFirstType },
    TSinkFirstType,
    TSinkPayloadType as IResultThriftStruct with { type TResult = TSinkType },
    TSinkType,
    TSinkFinalResponseType as IResultThriftStruct with {
      type TResult = TSinkFinalType },
    TSinkFinalType,
  >(
    classname<TSinkFirstResponseType> $first_response_type,
    class<TSinkPayloadType> $sink_payload_type,
    class<TSinkFinalResponseType> $final_response_type,
    string $name,
    bool $is_first_response_null,
    int $expectedsequenceid,
    RpcOptions $rpc_options,
    shape(?'read_options' => int) $options = shape(),
  ): Awaitable<
    \ResponseAndClientSink<TSinkFirstType, TSinkType, TSinkFinalType>,
  > {

    $channel = $this->channel_;
    $out_transport = $this->output_->getTransport();
    $in_transport = $this->input_->getTransport();
    invariant(
      $channel !== null &&
        $out_transport is \TMemoryBuffer &&
        $in_transport is \TMemoryBuffer,
      "Sink methods require nonnull channel and TMemoryBuffer transport",
    );

    $msg = $out_transport->getBuffer();
    $out_transport->resetBuffer();
    list($result_msg, $_read_headers, $sink) =
      await $channel->genSendRequestSink($rpc_options, $msg);

    $payload_serializer =
      ThriftStreamingSerializationHelpers::encodeStreamHelper(
        $sink_payload_type,
        $this->output_,
      );
    $final_response_deserializer =
      ThriftStreamingSerializationHelpers::decodeStreamHelper(
        $final_response_type,
        $name,
        $this->input_,
      );
    $client_sink_func = async function(
      AsyncGenerator<null, TSinkType, void> $pld_generator,
    ) use ($sink, $payload_serializer, $final_response_deserializer) {
      return await $sink->genSink<TSinkType, TSinkFinalType>(
        $pld_generator,
        $payload_serializer,
        $final_response_deserializer,
      );
    };

    $in_transport->resetBuffer();
    $in_transport->write($result_msg);
    $first_response = $this->recvImplHelper(
      $first_response_type,
      $name,
      $is_first_response_null,
      $expectedsequenceid,
      $options,
    );

    await $this->asyncHandler_
      ->genAfter<TSinkFirstType>($name, $first_response);
    return new \ResponseAndSink<TSinkFirstType, TSinkType, TSinkFinalType>(
      $first_response,
      $client_sink_func,
    );
  }
}
