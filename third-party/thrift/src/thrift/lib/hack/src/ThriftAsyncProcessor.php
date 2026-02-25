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

use namespace FlibSL\{C, Math, Str, Vec}; // @oss-enable
interface IThriftServiceMethodMetadata<TThriftIf as IThriftAsyncIf> {
  public function getArgsClass()[]: class<IThriftStruct>;
  public function genExecute<TArgs as IThriftStruct>(
    TThriftIf $handler,
    TArgs $args,
  ): Awaitable<void>;

  public function getResult(): ?IResultThriftStruct;
}

/**
 * Method metadata for oneway (fire-and-forget) methods.
 * Oneway methods do not send any response back to the client.
 * Unlike request-response methods, oneway methods don't need a result type
 * since they never send responses back to the client.
 */
final class ThriftServiceOnewayMethod<
  TThriftIf as IThriftAsyncIf,
  TArgs as IThriftStruct,
> implements IThriftServiceMethodMetadata<TThriftIf> {

  public function __construct(
    protected class<TArgs> $args,
    protected (function(TThriftIf, TArgs): Awaitable<void>) $handlerMethod,
  )[] {}

  public function getArgsClass()[]: class<TArgs> {
    return $this->args;
  }

  public async function genExecute(
    TThriftIf $handler,
    TArgs $args,
  ): Awaitable<void> {
    await ($this->handlerMethod)($handler, $args);
  }

  public function getResult(): ?IResultThriftStruct {
    return null;
  }
}

/**
 * Base class for method metadata returned by getMethodMetadata* functions.
 * Provides common interface for accessing args/result classes and handler methods.
 */
abstract class ThriftServiceMethod<
  TThriftIf as IThriftAsyncIf,
  TArgs as IThriftStruct,
  TResultStruct as IResultThriftStruct with { type TResult = TRet },
  TRet,
  THandlerResult,
> implements IThriftServiceMethodMetadata<TThriftIf> {
  protected ?THandlerResult $result = null;
  public function __construct(
    protected class<TArgs> $args,
    protected class<TResultStruct> $response,
    protected (function(
      TThriftIf,
      TArgs,
    ): Awaitable<THandlerResult>) $handlerMethod,
  )[] {}

  abstract protected function getSuccessResponse(): TRet;

  public async function genExecute(
    TThriftIf $handler,
    TArgs $args,
  ): Awaitable<void> {
    $this->result = await ($this->handlerMethod)($handler, $args);
  }

  public function getResult(): TResultStruct {
    $res_class = $this->response;
    $result_struct = $res_class::withDefaultValues();
    $this->setSuccessResponse($result_struct);
    return $result_struct;
  }

  // Hack typechecker fails if this is inlined in getResult.
  private function setSuccessResponse(TResultStruct $result_struct): void {
    if ($this->result === null) {
      return;
    }
    if ($result_struct is ThriftSyncStructWithResult) {
      $result_struct->success = $this->getSuccessResponse();
    } else if ($result_struct is ThriftAsyncStructWithResult) {
      $result_struct->success = $this->getSuccessResponse();
    }
  }

  public function getArgsClass()[]: class<TArgs> {
    return $this->args;
  }
}

/**
 * Method metadata for standard request-response methods.
 */
final class ThriftServiceRequestResponseMethod<
  TThriftIf as IThriftAsyncIf,
  TArgs as IThriftStruct,
  TResultStruct as IResultThriftStruct with { type TResult = TRet },
  TRet,
> extends ThriftServiceMethod<TThriftIf, TArgs, TResultStruct, TRet, TRet> {
  <<__Override>>
  protected function getSuccessResponse(): TRet {
    return $this->result as nonnull;
  }
}

/**
 * Method metadata for streaming response methods.
 */
final class ThriftServiceStreamingResponseMethod<
  TThriftIf as IThriftAsyncIf,
  TArgs as IThriftStruct,
  TResultStruct as IResultThriftStruct with { type TResult = TRet },
  TRet,
  TStreamPayloadStruct as IResultThriftStruct with {
    type TResult = TStreamType },
  TStreamType,
>
  extends ThriftServiceMethod<
    TThriftIf,
    TArgs,
    TResultStruct,
    TRet,
    ResponseAndStream<TRet, TStreamType>,
  > {

  public function __construct(
    protected class<TArgs> $args,
    protected class<TResultStruct> $ret,
    protected (function(
      TThriftIf,
      TArgs,
    ): Awaitable<ResponseAndStream<TRet, TStreamType>>) $handlerMethod,
    protected class<TStreamPayloadStruct> $streamPayload,
  )[] {
    parent::__construct($args, $ret, $handlerMethod);
  }

  <<__Override>>
  protected function getSuccessResponse(): TRet {
    $res_and_stream = $this->result as nonnull;
    return $res_and_stream->response as nonnull;
  }

  public function getStreamPayloadClass()[]: class<TStreamPayloadStruct> {
    return $this->streamPayload;
  }

  public function genStream()[]: HH\AsyncGenerator<null, TStreamType, void> {
    $this->result as nonnull;
    return $this->result->stream;
  }
}

/**
 * Method metadata for sink methods.
 */
final class ThriftServiceSinkResponseMethod<
  TThriftIf as IThriftAsyncIf,
  TArgs as IThriftStruct,
  TResultStruct as IResultThriftStruct with { type TResult = TRet },
  TRet,
  TSinkPayloadStruct as IResultThriftStruct with {
    type TResult = TSinkPayloadType },
  TSinkPayloadType,
  TFinalResponseStruct as IResultThriftStruct with {
    type TResult = TFinalResponseType },
  TFinalResponseType,
>
  extends ThriftServiceMethod<
    TThriftIf,
    TArgs,
    TResultStruct,
    TRet,
    ResponseAndSink<TRet, TSinkPayloadType, TFinalResponseType>,
  > {

  public function __construct(
    protected class<TArgs> $args,
    protected class<TResultStruct> $ret,
    protected (function(TThriftIf, TArgs): Awaitable<
      ResponseAndSink<TRet, TSinkPayloadType, TFinalResponseType>,
    >) $handlerMethod,
    protected class<TSinkPayloadStruct> $sinkPayload,
    protected class<TFinalResponseStruct> $finalResponse,
  )[] {
    parent::__construct($args, $ret, $handlerMethod);
  }

  <<__Override>>
  protected function getSuccessResponse(): TRet {
    $res_and_sink = $this->result as nonnull;
    return $res_and_sink->response as nonnull;
  }

  public function getSink()[]: (function(
    AsyncGenerator<null, TSinkPayloadType, void>,
  ): Awaitable<TFinalResponseType>) {
    $this->result as nonnull;
    return $this->result->genSink;
  }

  public function getSinkPayloadClass()[]: class<TSinkPayloadStruct> {
    return $this->sinkPayload;
  }

  public function getFinalResponseClass()[]: class<TFinalResponseStruct> {
    return $this->finalResponse;
  }
}

// @oss-disable: <<Oncalls('thrift')>>
abstract class ThriftAsyncProcessor
  extends ThriftProcessorBase
  implements IThriftAsyncProcessor {
  use GetThriftServiceMetadata;

  abstract const type TThriftIf as IThriftAsyncIf;
  const type TThriftServiceMethodMetadata =
    IThriftServiceMethodMetadata<this::TThriftIf>;

  use TWithMasBuenopathLastSet;

  /**
   * Returns the method metadata for the given method name, or null if the method is not supported.
   * This method should be overridden by generated service processors to provide metadata
   * for each supported method using a switch statement over all methods.
   *
   * @param string $fname The method name to get metadata for
   * @return The method metadata (ThriftServiceMethod or ThriftServiceOnewayMethod), or null if not supported
   */
  protected static function getMethodMetadata(
    string $_fname,
  ): ?this::TThriftServiceMethodMetadata {
    return null;
  }

  <<StringMetadataExtractor('Thrift:')>>
  final public async function processAsync(
    TProtocol $input,
    TProtocol $output,
    ?string $fname = null,
    ?int $rseqid = null,
  ): Awaitable<bool> {
    if ($fname === null || $rseqid === null) {
      $rseqid = 0;
      $fname = '';
      $mtype = 0;

      $input->readMessageBegin(inout $fname, inout $mtype, inout $rseqid);
    }

    HH\set_frame_metadata(nameof static.':'.$fname);
    if (!$this->isSubRequest()) {
      RelativeScript::setMinorPath($fname);
    }
    $methodname = 'process_'.$fname;
    if (JustKnobs::eval('www/mas_buenopath:enable_mbp_thrift')) {
      self::setBuenopath(ThriftMasBuenopath::newBuilder(
        shape("class_name" => static::class, "method_name" => $methodname),
      ));
    }
    // Check for new-style method metadata first (controlled by JustKnobs)
    $use_method_metadata = false;
    try {
      $use_method_metadata = JustKnobs::eval(
        'thrift/hack:thrift_use_method_metadata_processor',
        null,
        nameof static,
      );
    } catch (Exception $ex) {
      // Knob doesn't exist yet (propagation delay), default to false (use old code path)
      Ope::markClownyControlFlowException(
        $ex,
        'because JustKnobs knob may not exist yet during rollout',
      );
    }

    if ($use_method_metadata) {
      if ($fname === 'getThriftServiceMetadata') {
        // getThriftServiceMetadata is a special introspection method not in
        // getMethodMetadata. Handle it directly using the trait helper.
        $this->process_getThriftServiceMetadataHelper(
          $rseqid,
          $input,
          $output,
          static::SERVICE_METADATA_CLASS,
        );
        return true;
      }

      $method_metadata = static::getMethodMetadata($fname);
      if ($method_metadata is nonnull) {
        await $this->genProcessMethod(
          $method_metadata,
          $fname,
          $rseqid,
          $input,
          $output,
        );
        return true;
      }
    } else if ($this->isSupportedMethod($methodname)) {
      // Fall back to old-style process_ method for backward compatibility
      /* HH_FIXME[2011]: This is safe */
      // @lint-ignore DYNAMICALLY_INVOKING_TARGETS_CONSIDERED_HARMFUL
      await $this->$methodname($rseqid, $input, $output);
      return true;
    }

    $handler_ctx = $this->eventHandler_->getHandlerContext($fname);
    $this->eventHandler_->preRead($handler_ctx, $fname, dict[]);
    $input->skip(TType::STRUCT);
    $input->readMessageEnd();
    $this->eventHandler_->postRead($handler_ctx, $fname, dict[]);
    $x = TApplicationException::fromShape(shape(
      'message' => 'Function '.$fname.' not implemented.',
      'code' => TApplicationException::UNKNOWN_METHOD,
    ));
    $this->eventHandler_->handlerError($handler_ctx, $fname, $x);
    $output->writeMessageBegin($fname, TMessageType::EXCEPTION, $rseqid);
    $x->write($output);
    $output->writeMessageEnd();
    $output->getTransport()->flush();
    return true;
  }

  protected async function genProcessMethod(
    this::TThriftServiceMethodMetadata $method_metadata,
    string $fname,
    int $seqid,
    TProtocol $input,
    TProtocol $output,
  ): Awaitable<void> {

    $handler_ctx = $this->eventHandler_->getHandlerContext($fname);

    $args_class = $method_metadata->getArgsClass();
    $args = $this->readHelper($args_class, $input, $fname, $handler_ctx);
    $this->eventHandler_
      ->preExec($handler_ctx, static::THRIFT_SVC_NAME, $fname, $args);
    $reply_type = TMessageType::REPLY;
    try {
      await $method_metadata->genExecute($this->handler, $args);
      $result = $method_metadata->getResult();
      if ($result === null) {
        // oneway methods don't send any response back to the client
        // and don't invoke any other event handlers
        return;
      }

      $this->eventHandler_->postExec($handler_ctx, $fname, $result);
      $this->writeHelper(
        $result,
        $fname,
        $seqid,
        $handler_ctx,
        $output,
        $reply_type,
      );
      if (
        $method_metadata
          is ThriftServiceStreamingResponseMethod<_, _, _, _, _, _>
      ) {
        await $this->genExecuteStream(
          $method_metadata->genStream(),
          $method_metadata->getStreamPayloadClass(),
          $output,
          $fname,
          $handler_ctx,
        );
      } else if (
        $method_metadata
          is ThriftServiceSinkResponseMethod<_, _, _, _, _, _, _, _>
      ) {
        await $this->genExecuteSink(
          $method_metadata->getSink(),
          $method_metadata->getSinkPayloadClass(),
          $method_metadata->getFinalResponseClass(),
          $input,
          $output,
          $fname,
          $handler_ctx,
        );
      }
    } catch (Exception $ex) {
      $result = $method_metadata->getResult();
      // For oneway methods, log error but don't send response
      if ($result === null) {
        $this->eventHandler_->handlerError($handler_ctx, $fname, $ex);
        return;
      }
      if ($result->setException($ex)) {
        $this->eventHandler_->handlerException($handler_ctx, $fname, $ex);
      } else {
        $reply_type = TMessageType::EXCEPTION;
        $this->eventHandler_->handlerError($handler_ctx, $fname, $ex);
        $result = new TApplicationException(
          $ex->getMessage()."\n".$ex->getTraceAsString(),
        );
      }
      $this->writeHelper(
        $result,
        $fname,
        $seqid,
        $handler_ctx,
        $output,
        $reply_type,
      );
    }
  }

  public function process(
    TProtocol $input,
    TProtocol $output,
    ?string $fname = null,
    ?int $rseqid = null,
  ): bool {
    return Asio::awaitSynchronously(
      $this->processAsync($input, $output, $fname, $rseqid),
    );
  }
}
