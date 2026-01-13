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

/**
 * Base class for method metadata returned by getMethodMetadata* functions.
 * Provides common interface for accessing args/result classes and handler methods.
 */
abstract class ThriftServiceMethod<
  TThriftIf as IThriftAsyncIf,
  TArgs as IThriftStruct,
  TResultStruct as IResultThriftStruct,
  THandlerResult,
> {
  abstract const Thrift_RpcMetadata_RpcKind RPC_KIND;

  public function __construct(
    protected classname<TArgs> $args,
    protected classname<TResultStruct> $ret,
    protected (function(
      TThriftIf,
      TArgs,
    ): Awaitable<THandlerResult>) $handlerMethod,
  )[] {}

  abstract public function genResult(
    TThriftIf $handler,
    TArgs $args,
    TResultStruct $res,
  ): Awaitable<THandlerResult>;

  public function getArgsClass()[]: classname<TArgs> {
    return $this->args;
  }

  public function getResultClass()[]: classname<TResultStruct> {
    return $this->ret;
  }

  public function isStreaming()[]: bool {
    return false;
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
> extends ThriftServiceMethod<TThriftIf, TArgs, TResultStruct, TRet> {
  const Thrift_RpcMetadata_RpcKind RPC_KIND =
    Thrift_RpcMetadata_RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;

  <<__Override>>
  public async function genResult(
    TThriftIf $handler,
    TArgs $args,
    TResultStruct $result,
  ): Awaitable<TRet> {
    $response = await ($this->handlerMethod)($handler, $args);
    if ($result is ThriftSyncStructWithResult) {
      $result->success = $response;
    } else if ($result is ThriftAsyncStructWithResult) {
      $result->success = $response;
    }
    return $response;
  }
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
> {
  const Thrift_RpcMetadata_RpcKind RPC_KIND =
    Thrift_RpcMetadata_RpcKind::SINGLE_REQUEST_NO_RESPONSE;

  public function __construct(
    protected classname<TArgs> $args,
    protected (function(TThriftIf, TArgs): Awaitable<void>) $handlerMethod,
  )[] {}

  public function getArgsClass()[]: classname<TArgs> {
    return $this->args;
  }

  public async function genExecute(
    TThriftIf $handler,
    TArgs $args,
  ): Awaitable<void> {
    await ($this->handlerMethod)($handler, $args);
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
    ResponseAndStream<TRet, TStreamType>,
  > {
  const Thrift_RpcMetadata_RpcKind RPC_KIND =
    Thrift_RpcMetadata_RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE;

  public function __construct(
    protected classname<TArgs> $args,
    protected classname<TResultStruct> $ret,
    protected (function(
      TThriftIf,
      TArgs,
    ): Awaitable<ResponseAndStream<TRet, TStreamType>>) $handlerMethod,
    protected classname<TStreamPayloadStruct> $streamPayload,
  )[] {
    parent::__construct($args, $ret, $handlerMethod);
  }

  <<__Override>>
  public async function genResult(
    TThriftIf $handler,
    TArgs $args,
    TResultStruct $result,
  ): Awaitable<ResponseAndStream<TRet, TStreamType>> {
    $res_and_stream = await ($this->handlerMethod)($handler, $args);
    if ($result is ThriftSyncStructWithResult) {
      $result->success = $res_and_stream->response;
    } else if ($result is ThriftAsyncStructWithResult) {
      $result->success = $res_and_stream->response;
    }
    return $res_and_stream;
  }

  public function getStreamPayloadClass()[]: classname<IResultThriftStruct> {
    return $this->streamPayload;
  }

  <<__Override>>
  public function isStreaming()[]: bool {
    return true;
  }
}

/**
 * Method metadata for sink methods.
 */
final class ThriftServiceSinkMethod<
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
    ResponseAndSink<TRet, TSinkPayloadType, TFinalResponseType>,
  > {
  const Thrift_RpcMetadata_RpcKind RPC_KIND = Thrift_RpcMetadata_RpcKind::SINK;

  public function __construct(
    protected classname<TArgs> $args,
    protected classname<TResultStruct> $ret,
    protected (function(TThriftIf, TArgs): Awaitable<
      ResponseAndSink<TRet, TSinkPayloadType, TFinalResponseType>,
    >) $handlerMethod,
    protected classname<TSinkPayloadStruct> $sinkPayload,
    protected classname<TFinalResponseStruct> $finalResponse,
  )[] {
    parent::__construct($args, $ret, $handlerMethod);
  }

  <<__Override>>
  public async function genResult(
    TThriftIf $handler,
    TArgs $args,
    TResultStruct $result,
  ): Awaitable<ResponseAndSink<TRet, TSinkPayloadType, TFinalResponseType>> {
    $response_and_sink = await ($this->handlerMethod)($handler, $args);
    if ($result is ThriftSyncStructWithResult) {
      $result->success = $response_and_sink->response;
    } else if ($result is ThriftAsyncStructWithResult) {
      $result->success = $response_and_sink->response;
    }
    return $response_and_sink;
  }

  public function getSinkPayloadClass()[]: classname<IResultThriftStruct> {
    return $this->sinkPayload;
  }

  public function getFinalResponseClass()[]: classname<IResultThriftStruct> {
    return $this->finalResponse;
  }

  public function isSink()[]: bool {
    return true;
  }
}

// @oss-disable: <<Oncalls('thrift')>>
abstract class ThriftAsyncProcessor
  extends ThriftProcessorBase
  implements IThriftAsyncProcessor {

  abstract const type TThriftIf as IThriftAsyncIf;

  use TWithMasBuenopathLastSet;

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
    if (!$this->isSupportedMethod($methodname)) {
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
    /* HH_FIXME[2011]: This is safe */
    await $this->$methodname($rseqid, $input, $output);
    return true;
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
