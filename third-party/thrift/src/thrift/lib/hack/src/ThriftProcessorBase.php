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

<<Oncalls('thrift')>> // @oss-disable
abstract class ThriftProcessorBase implements IThriftProcessor {
  abstract const type TThriftIf as IThriftIf;
  abstract const string THRIFT_SVC_NAME;
  protected TProcessorEventHandler $eventHandler_;
  private bool $isSubRequest = false;
  private vec<int> $flushTimes = vec[];

  public function __construct(protected this::TThriftIf $handler)[] {
    $this->eventHandler_ = new TProcessorEventHandler();
  }

  public function getHandler()[]: this::TThriftIf {
    return $this->handler;
  }

  public function setEventHandler(
    TProcessorEventHandler $event_handler,
  )[write_props]: this {
    $this->eventHandler_ = $event_handler;
    return $this;
  }

  public function getEventHandler()[]: TProcessorEventHandler {
    return $this->eventHandler_;
  }

  final public function setIsSubRequest(
    bool $is_sub_request = true,
  )[write_props]: this {
    $this->isSubRequest = $is_sub_request;
    return $this;
  }

  final public function isSubRequest()[]: bool {
    return $this->isSubRequest;
  }

  /**
   * Helper method to be used in the generated {Sync,Async}ProcessorBase classes
   */
  final protected function readHelper<TResult as IThriftStruct>(
    classname<TResult> $request_args_class,
    \TProtocol $input,
    string $request_name,
    mixed $handler_ctx,
  ): dynamic {
    $this->eventHandler_->preRead($handler_ctx, $request_name, dict[]);

    if ($input is \TBinaryProtocolAccelerated) {
      $args = thrift_protocol_read_binary_struct(
        $input,
        HH\class_to_classname($request_args_class),
      );
    } else if ($input is \TCompactProtocolAccelerated) {
      $args = thrift_protocol_read_compact_struct(
        $input,
        HH\class_to_classname($request_args_class),
      );
    } else {
      $args = $request_args_class::withDefaultValues();
      $args->read($input);
    }
    $input->readMessageEnd();
    $this->eventHandler_->postRead($handler_ctx, $request_name, $args);

    return $args;
  }

  /**
   * Helper method to be used in the generated {Sync,Async}ProcessorBase classes
   */
  final protected function writeHelper<TResult as IThriftStruct>(
    TResult $result,
    string $request_name,
    int $seqid,
    mixed $handler_ctx,
    \TProtocol $output,
    TMessageType $reply_type,
  ): void {
    $this->eventHandler_->preWrite($handler_ctx, $request_name, $result);
    if ($output is \TBinaryProtocolAccelerated) {
      thrift_protocol_write_binary(
        $output,
        $request_name,
        $reply_type,
        $result,
        $seqid,
        $output->isStrictWrite(),
      );
    } else if ($output is \TCompactProtocolAccelerated) {
      thrift_protocol_write_compact2(
        $output,
        $request_name,
        $reply_type,
        $result,
        $seqid,
        false,
        TCompactProtocolBase::VERSION,
      );
    } else {
      $output->writeMessageBegin($request_name, $reply_type, $seqid);
      $result->write($output);
      $output->writeMessageEnd();
      $output->getTransport()->flush();
    }
    $this->eventHandler_->postWrite($handler_ctx, $request_name, $result);
  }

  final protected async function genExecuteStream<
    TStreamResponseType as IResultThriftStruct with {
      type TResult = TStreamType },
    TStreamType,
  >(
    HH\AsyncGenerator<null, TStreamType, void> $stream,
    class<TStreamResponseType> $stream_response_type,
    \TProtocol $output,
    string $_request_name = '',
    mixed $_handler_ctx = null,
  ): Awaitable<void> {
    $transport = $output->getTransport();
    invariant(
      $transport is \TMemoryBuffer,
      "Stream methods require TMemoryBuffer transport",
    );
    $encoded_first_response = $transport->getBuffer();
    $transport->resetBuffer();
    $server_stream = await PHP\gen_start_thrift_stream($encoded_first_response);
    // Set fatal error handling for thrift streaming requests.
    // This will send proper thrift exception back to the caller
    // instead of default html page for 500.
    PHP\hphp_set_error_page(
      php_root().
      '/flib/core/runtime/error/error_pages/thrift/handle_thrift_streaming_service_error.php',
    );
    try {
      $payload_encoder =
        $this->encodeStreamWithLatencyTracking($stream_response_type, $output);
      if ($server_stream === null) {
        // Stream was cancelled by the client.
        return;
      }
      await $this->genStream<TStreamType>(
        $server_stream,
        $stream,
        $payload_encoder,
      );
    } catch (Exception $ex) {
      $payload_encoder =
        ThriftStreamingSerializationHelpers::encodeStreamHelper(
          $stream_response_type,
          $output,
        );
      list($serialized_ex, $is_tax) = $payload_encoder(null, $ex);
      $server_stream?->sendServerException(
        $serialized_ex,
        $ex->getMessage(),
        get_class($ex),
        !$is_tax,
      );
    }
  }

  final protected function encodeStreamWithLatencyTracking<
    TStreamPayloadType as IResultThriftStruct with {
      type TResult = TStreamType },
    TStreamType,
  >(
    class<TStreamPayloadType> $stream_response_type,
    TProtocol $output,
  ): (function(?TStreamType, ?Exception): (string, bool)) {
    $payload_encoder = ThriftStreamingSerializationHelpers::encodeStreamHelper(
      $stream_response_type,
      $output,
    );
    return (?TStreamType $payload, ?\Exception $ex) ==> {
      $this->flushTimes[] = request_stats_total_wall();
      if (C\count($this->flushTimes) === 1) {
        $flush0 = $this->flushTimes[0];
        PerfMetadata::get()->setThriftFlush0Duration($flush0);
      }
      return $payload_encoder($payload, $ex);
    };
  }

  final protected async function genStream<TStreamType>(
    \TServerStream $server_stream,
    HH\AsyncGenerator<null, TStreamType, void> $payload_generator,
    (function(?TStreamType, ?Exception): (string, ?bool)) $payload_encode,
  ): Awaitable<void> {
    $should_continue = false;
    while ($should_continue !== null) {
      if ($should_continue === false) {
        // wait till stream resumes or credits are received;
        $should_continue = await $server_stream->genIsStreamReady();
        continue;
      }

      try {
        $item = await $payload_generator->next();
        if ($item === null) {
          // send stream complete and return.
          return $server_stream->sendStreamComplete();
        }
        list($_, $pld) = $item;

        list($encoded_str, $_) = $payload_encode($pld, null);
        $should_continue =
          await $server_stream->genSendStreamPayload($encoded_str);

      } catch (Exception $ex) {
        // If async generator throws any error,
        // exception should be encoded and sent to client before throwing
        list($encoded_ex, $is_application_ex) = $payload_encode(null, $ex);

        $server_stream->sendServerException(
          $encoded_ex,
          $ex->getMessage(),
          Classnames::get($ex) as nonnull,
          !$is_application_ex,
        );
        return;
      }
    }
  }

  final public function isSupportedMethod(string $fname_with_prefix)[]: bool {
    return PHP\method_exists($this, $fname_with_prefix);
  }
}

/**
 * This trait defines the `process_getThriftServiceMetadataHelper()` method.
 * This method is used to implement the `process_getThriftServiceMetadata()`
 * method in {ServiceName}{Async|Sync}ProcessorBase classes.
 */
trait GetThriftServiceMetadata {
  require extends ThriftProcessorBase;
  require implements IThriftProcessor;

  private function process_getThriftServiceMetadataHelper(
    int $seqid,
    \TProtocol $input,
    \TProtocol $output,
    classname<\IThriftServiceStaticMetadata> $service_metadata_class,
  ): void {
    $reply_type = TMessageType::REPLY;

    if ($input is \TBinaryProtocolAccelerated) {
      thrift_protocol_read_binary_struct(
        $input,
        '\tmeta_ThriftMetadataService_getThriftServiceMetadata_args',
      );
    } else if ($input is \TCompactProtocolAccelerated) {
      thrift_protocol_read_compact_struct(
        $input,
        '\tmeta_ThriftMetadataService_getThriftServiceMetadata_args',
      );
    } else {
      $args =
        tmeta_ThriftMetadataService_getThriftServiceMetadata_args::withDefaultValues();
      $args->read($input);
    }
    $input->readMessageEnd();
    $result =
      tmeta_ThriftMetadataService_getThriftServiceMetadata_result::withDefaultValues();
    try {
      $result->success = $service_metadata_class::getServiceMetadataResponse();
    } catch (\Exception $ex) {
      $reply_type = TMessageType::EXCEPTION;
      $result = new \TApplicationException(
        $ex->getMessage()."\n".$ex->getTraceAsString(),
      );
    }
    if ($output is \TBinaryProtocolAccelerated) {
      thrift_protocol_write_binary(
        $output,
        'getThriftServiceMetadata',
        $reply_type,
        $result,
        $seqid,
        $output->isStrictWrite(),
      );
    } else if ($output is \TCompactProtocolAccelerated) {
      thrift_protocol_write_compact2(
        $output,
        'getThriftServiceMetadata',
        $reply_type,
        $result,
        $seqid,
        false,
        TCompactProtocolBase::VERSION,
      );
    } else {
      $output->writeMessageBegin(
        "getThriftServiceMetadata",
        $reply_type,
        $seqid,
      );
      $result->write($output);
      $output->writeMessageEnd();
      $output->getTransport()->flush();
    }
  }
}
