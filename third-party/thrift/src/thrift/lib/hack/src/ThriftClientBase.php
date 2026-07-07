<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
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

use namespace FlibSL\{C, Math, Str, Vec}; // @oss-enable

type ThriftClientFactory<T> = (classname<T>, (function(
  TProtocol,
  ?TProtocol,
  ?IThriftMigrationAsyncChannel,
): T));

// @oss-disable: <<__ConsistentConstruct, Oncalls('thrift')>>
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

  <<__NeedsConcrete, __DynamicallyCallable>>
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
    $this->output_ = $output is nonnull ? $output : $input;
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
      // Generated thrift clients pass $is_one_way=true for `oneway void`
      // IDL methods. Notify handlers so the request edge gets annotated
      // (see ThriftContextPropClientEventHandler::postSendHelper).
      if ($is_one_way) {
        $this->eventHandler_->markCallAsOneway($currentseqid);
      }
      $this->eventHandler_
        ->preSend($function_name, $args, $currentseqid, $service_name);
      $this->output_->writeRPCMessage(
        $function_name,
        TMessageType::CALL,
        $args,
        $currentseqid,
        $is_one_way,
      );
    } catch (THandlerShortCircuitException $ex) {
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
    } catch (Exception $ex) {
      $this->eventHandler_
        ->sendError($function_name, $args, $currentseqid, $ex);
      throw $ex;
    }
    $this->eventHandler_->postSend($function_name, $args, $currentseqid);
    return $currentseqid;
  }

  private async function genInvokeOnErrorHook(
    string $name,
    Exception $recv_exception,
  ): Awaitable<void> {
    try {
      await $this->asyncHandler_->genOnError($name, $recv_exception);
    } catch (Exception $hook_exception) {
      ope($hook_exception, causes_the('follow-up recv hook')->to('be skipped'));
    }
  }

  protected async function genRecvImplHelper<
    TResulttype as IResultThriftStruct with { type TResult = TRet },
    TRet,
  >(
    classname<TResulttype> $result,
    string $name,
    bool $is_return_void,
    ?int $expectedsequenceid,
    shape(?'read_options' => int) $options = shape(),
  ): Awaitable<TRet> {
    try {
      $this->eventHandler_->preRecv($name, $expectedsequenceid);
      $result = $this->input_->readRPCMessage(
        $result,
        $name,
        $expectedsequenceid,
        Shapes::idx($options, 'read_options', 0),
      );
    } catch (THandlerShortCircuitException $ex) {
      switch ($ex->resultType) {
        case THandlerShortCircuitException::R_EXPECTED_EX:
          $this->eventHandler_
            ->recvException($name, $expectedsequenceid, $ex->result);
          await $this->genInvokeOnErrorHook($name, $ex->result);
          throw $ex->result;
        case THandlerShortCircuitException::R_UNEXPECTED_EX:
          $this->eventHandler_
            ->recvError($name, $expectedsequenceid, $ex->result);
          await $this->genInvokeOnErrorHook($name, $ex->result);
          throw $ex->result;
        case THandlerShortCircuitException::R_SUCCESS:
        default:
          $this->eventHandler_
            ->postRecv($name, $expectedsequenceid, $ex->result);
          // this should just always be null in the ThriftSyncStructWithoutResult case
          return $ex->result;
      }
    } catch (Exception $ex) {
      $this->eventHandler_->recvError($name, $expectedsequenceid, $ex);
      await $this->genInvokeOnErrorHook($name, $ex);
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
      await $this->genInvokeOnErrorHook($name, $exception);
      throw $exception;
    }

    if ($is_return_void) {
      $this->eventHandler_
        ->postRecv($name, $expectedsequenceid, null);
      return
        HH\FIXME\UNSAFE_CAST<null, TRet>(null, 'FIXME[4110] TRet is void here');
    } else {
      $x = new TApplicationException(
        $name." failed: unknown result",
        TApplicationException::MISSING_RESULT,
      );
      $this->eventHandler_
        ->recvError($name, $expectedsequenceid, $x);
      await $this->genInvokeOnErrorHook($name, $x);
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
      $out_transport is TMemoryBuffer &&
      $in_transport is TMemoryBuffer
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
    $response = await $this->genRecvImplHelper(
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
    if ($channel !== null && $out_transport is TMemoryBuffer) {
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
  ): Awaitable<ResponseAndStream<TFirstType, TStreamType>> {

    $channel = $this->channel_;
    $out_transport = $this->output_->getTransport();
    $in_transport = $this->input_->getTransport();
    invariant(
      $out_transport is TMemoryBuffer && $in_transport is TMemoryBuffer,
      "Stream methods require TMemoryBuffer transport",
    );
    $decoder = ThriftStreamingSerializationHelpers::decodeStreamHelper(
      $stream_response_type,
      $name,
      $this->input_,
      $options,
    );

    if ($channel !== null) {
      $msg = $out_transport->getBuffer();
      $out_transport->resetBuffer();
      list($result_msg, $_read_headers, $stream) =
        await $channel->genSendRequestStreamResponse($rpc_options, $msg);
      $disable16kblimit = $this->config_?->getStreamDisable16KBLimit() ?? false;
      if ($disable16kblimit) {
        $stream->disable16KBBufferingPolicy();
      }

      $stream_gen = $stream->gen<TStreamType>($decoder);
      $in_transport->resetBuffer();
      $in_transport->write($result_msg);
    } else {
      $raw_stream = await $this->asyncHandler_
        ->genWaitStream($expectedsequenceid);
      $stream_gen = (
        async function() use ($raw_stream, $decoder) {
          foreach ($raw_stream await as $raw_payload) {
            yield $decoder($raw_payload, null);
          }
        }
      )();
    }
    $first_response = await $this->genRecvImplHelper(
      $first_response_type,
      $name,
      $is_first_response_null,
      $expectedsequenceid,
      $options,
    );

    await $this->asyncHandler_
      ->genAfter<TFirstType>($name, $first_response);

    $handler = $this->asyncHandler_;
    $use_stream_hooks =
      JustKnobs::eval('www/thrift:use_stream_async_handler_hooks');
    $wrapped_stream = $use_stream_hooks
      ? (
          async function() use ($stream_gen, $handler, $name) {
            do {
              // @lint-ignore-every AWAIT_IN_LOOP chunks are processed
              // sequentially from the stream
              await $handler->genBeforeStream($name);
              // This wall time operation is required for low level IO calls or errors
              // will be logged or possibly thrown. Do not put more than one statement in this try block
              // TODO: this WTO (and the follow-up incrDuration) would ideally
              // live in TServiceRouterChannel::genSendRequestStreamResponse
              // (close to the raw I/O source), so the SR service name is
              // available for dynostats attribution and non-SR channels (e.g.
              // TFaasStatefulAsyncHandler::genWaitStream) don't pay for
              // wrapping. Would require a www wrapper around
              // TAsyncChannelStreamResponse whose gen() wraps each next() in a
              // (SR)WallTimeOperation. See D110460989 review discussion.
              $timer = WallTimeOperation::begin();
              try {
                $next = await $stream_gen->next();
              } finally {
                $timer->end();
                ProfilingCounters::incrDuration(
                  ProfilingCounterDuration::THRIFT_READ_DURATION,
                  $timer->getOldestRunningIODuration(),
                );
              }
              if ($next !== null) {
                list($_, $chunk) = $next;
                await $handler->genAfterStream<TStreamType>($name, $chunk);
                yield $chunk;
              }
            } while ($next !== null);
          }
        )()
      : $stream_gen;

    return new ResponseAndStream<TFirstType, TStreamType>(
      $first_response,
      $wrapped_stream,
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
  ): Awaitable<ResponseAndSink<TSinkFirstType, TSinkType, TSinkFinalType>> {

    $channel = $this->channel_;
    $out_transport = $this->output_->getTransport();
    $in_transport = $this->input_->getTransport();
    invariant(
      $out_transport is TMemoryBuffer && $in_transport is TMemoryBuffer,
      "Sink methods require MemoryBuffer transport",
    );

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

    if ($channel !== null) {
      $msg = $out_transport->getBuffer();
      $out_transport->resetBuffer();
      list($result_msg, $_read_headers, $sink) =
        await $channel->genSendRequestSink($rpc_options, $msg);
      $client_sink_func = async function(
        AsyncGenerator<null, TSinkType, void> $pld_generator,
      ) use ($sink, $payload_serializer, $final_response_deserializer) {
        $final_response = await $this->genSinkFunc<TSinkType>(
          $sink,
          $pld_generator,
          $payload_serializer,
        );
        return $final_response_deserializer($final_response, null);
      };

      $in_transport->resetBuffer();
      $in_transport->write($result_msg);
    } else {
      $sink_fn = await $this->asyncHandler_->genWaitSink($expectedsequenceid);
      $client_sink_func = async function(
        AsyncGenerator<null, TSinkType, void> $pld_generator,
      ) use ($sink_fn, $payload_serializer, $final_response_deserializer) {
        $raw_gen = (
          async function() use ($pld_generator, $payload_serializer) {
            foreach ($pld_generator await as $payload) {
              list($raw_bytes, $_is_app_ex) =
                $payload_serializer($payload, null);
              yield $raw_bytes;
            }
          }
        )();
        $final_raw = await $sink_fn($raw_gen);
        return $final_response_deserializer($final_raw, null);
      };
    }
    $first_response = await $this->genRecvImplHelper(
      $first_response_type,
      $name,
      $is_first_response_null,
      $expectedsequenceid,
      $options,
    );

    await $this->asyncHandler_
      ->genAfter<TSinkFirstType>($name, $first_response);
    return new ResponseAndSink<TSinkFirstType, TSinkType, TSinkFinalType>(
      $first_response,
      $client_sink_func,
    );
  }

  protected async function genAwaitBiDiStreamResponse<
    TBiDiFirstResponseType as IResultThriftStruct with {
      type TResult = TBiDiFirstType },
    TBiDiFirstType,
    TBiDiSinkPayloadType as IResultThriftStruct with {
      type TResult = TBiDiSinkType },
    TBiDiStreamResponseType as IResultThriftStruct with {
      type TResult = TBiDiStreamType },
    TBiDiStreamType,
    TBiDiSinkType,
  >(
    classname<TBiDiFirstResponseType> $first_response_type,
    class<TBiDiSinkPayloadType> $sink_payload_type,
    class<TBiDiStreamResponseType> $stream_response_type,
    string $name,
    bool $is_first_response_null,
    int $expectedsequenceid,
    RpcOptions $rpc_options,
    shape(?'read_options' => int) $options = shape(),
  ): Awaitable<
    ResponseAndBidirectionalStream<
      TBiDiFirstType,
      TBiDiSinkType,
      TBiDiStreamType,
    >,
  > {

    $channel = $this->channel_;
    $out_transport = $this->output_->getTransport();
    $in_transport = $this->input_->getTransport();
    invariant(
      $out_transport is TMemoryBuffer && $in_transport is TMemoryBuffer,
      "BiDi stream methods require TMemoryBuffer transport",
    );

    $stream_decoder = ThriftStreamingSerializationHelpers::decodeStreamHelper(
      $stream_response_type,
      $name,
      $this->input_,
      $options,
    );

    $payload_serializer =
      ThriftStreamingSerializationHelpers::encodeStreamHelper(
        $sink_payload_type,
        $this->output_,
      );

    if ($channel !== null) {
      $msg = $out_transport->getBuffer();
      $out_transport->resetBuffer();
      list($result_msg, $_read_headers, $sink, $stream) =
        await $channel->genSendRequestBiDiStream($rpc_options, $msg);

      $disable16kblimit = $this->config_?->getStreamDisable16KBLimit() ?? false;
      if ($disable16kblimit) {
        $stream->disable16KBBufferingPolicy();
      }

      $stream_gen = $stream->gen<TBiDiStreamType>($stream_decoder);

      $client_sink_func = async function(
        AsyncGenerator<null, TBiDiSinkType, void> $pld_generator,
      ) use ($sink, $payload_serializer) {
        await $this->genSinkFunc<TBiDiSinkType>(
          $sink,
          $pld_generator,
          $payload_serializer,
          true,
        );
      };

      $in_transport->resetBuffer();
      $in_transport->write($result_msg);
    } else {
      list($raw_stream, $raw_sink_func) = await $this->asyncHandler_
        ->genWaitBiDi($expectedsequenceid);

      $stream_gen = (
        async function() use ($raw_stream, $stream_decoder) {
          foreach ($raw_stream await as $raw_payload) {
            yield $stream_decoder($raw_payload, null);
          }
        }
      )();

      $client_sink_func = async function(
        AsyncGenerator<null, TBiDiSinkType, void> $pld_generator,
      ) use ($raw_sink_func, $payload_serializer) {
        $raw_gen = (
          async function() use ($pld_generator, $payload_serializer) {
            foreach ($pld_generator await as $item) {
              list($serialized, $_) = $payload_serializer($item, null);
              yield $serialized;
            }
          }
        )();
        await $raw_sink_func($raw_gen);
      };
    }

    $first_response = await $this->genRecvImplHelper(
      $first_response_type,
      $name,
      $is_first_response_null,
      $expectedsequenceid,
      $options,
    );

    await $this->asyncHandler_
      ->genAfter<TBiDiFirstType>($name, $first_response);
    return new ResponseAndBidirectionalStream<
      TBiDiFirstType,
      TBiDiSinkType,
      TBiDiStreamType,
    >($first_response, $client_sink_func, $stream_gen);
  }

  protected async function genSinkFunc<TSinkType>(
    TClientSink $sink,
    HH\AsyncGenerator<null, TSinkType, void> $pld_generator,
    (function(?TSinkType, ?Exception): (string, ?bool)) $payload_serializer,
    bool $skip_final_response = false,
  ): Awaitable<?string> {
    $should_continue = true;
    $gen_credits_or_final_response_helper = async () ==> {
      list($credits, $final_response, $exception) = HH\FIXME\UNSAFE_CAST<
        ?(int, ?string, ?string),
        (?int, ?string, ?string),
      >(await $sink->genCreditsOrFinalResponse());
      if (
        ($credits === null || $credits === 0) &&
        $final_response === null &&
        $exception === null
      ) {
        $exception = "No credits or final response received";
      }
      return tuple(
        $credits,
        $final_response,
        $exception !== null
          ? new TApplicationException(
              $exception,
              TApplicationException::UNKNOWN,
            )
          : null,
      );
    };
    while (true) {
      list($credits, $final_response, $exception) =
        await $gen_credits_or_final_response_helper();
      if ($final_response !== null || $exception !== null) {
        break;
      }
      $credits = HH\FIXME\UNSAFE_CAST<?int, int>($credits);
      if ($credits > 0 && $should_continue) {
        try {
          foreach ($pld_generator await as $pld) {
            list($encoded_str, $_) = $payload_serializer($pld, null);
            $should_continue = $sink->sendPayloadOrSinkComplete($encoded_str);
            $credits--;
            if ($credits === 0 || !$should_continue) {
              break;
            }
          }
        } catch (Exception $ex) {
          // If async generator throws any error,
          // exception should be encoded and sent to server before throwing
          list($encoded_ex, $is_application_ex) =
            $payload_serializer(null, $ex);
          $sink->sendClientException(
            $encoded_ex,
            ($is_application_ex ?? false) ? $ex->getMessage() : null,
          );
          // send exception back to the client, don't wait for final response
          throw $ex;
        }
        // If $credits > 0 and $should_continue = true,
        // then that means async generator has finished
        // and we should send sink complete.
        if ($credits > 0 && $should_continue) {
          $sink->sendPayloadOrSinkComplete(null);
          $should_continue = false;
          // For bidi, the server responds via the stream channel, not via
          // a sink final response. Return immediately after sink-complete.
          if ($skip_final_response) {
            return null;
          }
        }
      }
    }
    if ($exception !== null) {
      throw $exception;
    }
    return $final_response;
  }
}
