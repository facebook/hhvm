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
 */

/**
 * Async handler that simulates a streaming/sink server for testing.
 *
 * Follows the TFaasStatefulAsyncHandler pattern: shares protocols with the
 * thrift client, reads requests from the send transport, writes first
 * responses to the recv transport.
 */
<<Oncalls('thrift_hack')>>
final class TestStreamSinkAsyncHandler extends TClientAsyncHandler {

  private ?vec<string> $streamPayloads = null;
  private ?(function(vec<string>): example_ResponseStruct) $sinkProcessor =
    null;

  public function __construct(
    private TProtocol $recvProtocol,
    private TProtocol $sendProtocol,
    private example_ResponseStruct $firstResponse,
  ) {}

  public function setStreamPayloads(vec<string> $payloads): void {
    $this->streamPayloads = $payloads;
  }

  public function setSinkProcessor(
    (function(vec<string>): example_ResponseStruct) $processor,
  ): void {
    $this->sinkProcessor = $processor;
  }

  private function consumeRequestAndWriteFirstResponse(
    string $first_response_bytes,
  ): void {
    $transport = $this->sendProtocol->getTransport() as TMemoryBuffer;
    $method_name = '';
    $mtype = 0;
    $seqid = 0;
    $this->sendProtocol
      ->readMessageBegin(inout $method_name, inout $mtype, inout $seqid);
    $transport->resetBuffer();

    $this->recvProtocol
      ->writeMessageBegin($method_name, TMessageType::REPLY, 0);
    $this->recvProtocol->getTransport()->write($first_response_bytes);
    $this->recvProtocol->writeMessageEnd();
    $this->recvProtocol->getTransport()->flush();
  }

  <<__Override>>
  public async function genWaitStream(
    int $_sequence_id,
  )[zoned_local]: Awaitable<HH\AsyncGenerator<null, string, void>> {
    $first_response_struct =
      example_ExampleStreamingService_testStream_FirstResponse::fromShape(
        shape('success' => $this->firstResponse),
      );
    $this->consumeRequestAndWriteFirstResponse(
      TCompactSerializer::serialize($first_response_struct),
    );

    $payloads = $this->streamPayloads ?? vec[];
    $encoder = ThriftStreamingSerializationHelpers::encodeStreamHelper(
      example_ExampleStreamingService_testStream_StreamResponse::class,
      new TCompactProtocolAccelerated(new TMemoryBuffer()),
    );

    return (
      async function()[zoned_local] use ($payloads, $encoder) {
        foreach ($payloads as $payload) {
          list($raw_bytes, $_is_app_ex) = $encoder($payload, null);
          yield $raw_bytes;
        }
      }
    )();
  }

  <<__Override>>
  public async function genWaitSink(int $_sequence_id)[zoned_local]: Awaitable<
    (function(HH\AsyncGenerator<null, string, void>): Awaitable<string>),
  > {
    $first_response_struct =
      example_ExampleStreamingService_testSink_FirstResponse::fromShape(
        shape('success' => $this->firstResponse),
      );
    $this->consumeRequestAndWriteFirstResponse(
      TCompactSerializer::serialize($first_response_struct),
    );

    $sink_processor = $this->sinkProcessor;
    $decoder = ThriftStreamingSerializationHelpers::decodeStreamHelper(
      example_ExampleStreamingService_testSink_SinkPayload::class,
      'testSink',
      new TCompactProtocolAccelerated(new TMemoryBuffer()),
    );

    $encoder = ThriftStreamingSerializationHelpers::encodeStreamHelper(
      example_ExampleStreamingService_testSink_FinalResponse::class,
      new TCompactProtocolAccelerated(new TMemoryBuffer()),
    );

    return async (HH\AsyncGenerator<null, string, void> $gen)[zoned_local] ==> {
      $received_payloads = vec[];
      foreach ($gen await as $raw_payload) {
        $received_payloads[] = $decoder($raw_payload, null);
      }
      $final_response = example_ResponseStruct::fromShape(shape(
        'text' => 'done',
      ));
      if ($sink_processor is nonnull) {
        $final_response = $sink_processor($received_payloads);
      }
      list($raw_bytes, $_is_app_ex) = $encoder($final_response, null);
      return $raw_bytes;
    };
  }
}

/**
 * Lightweight handler that only yields stream payloads (no transport I/O).
 * Used as a secondary handler in TClientMultiAsyncHandler tests.
 */
<<Oncalls('thrift_hack')>>
final class TestStreamPayloadOnlyHandler extends TClientAsyncHandler {

  public function __construct(private vec<string> $streamPayloads) {}

  <<__Override>>
  public async function genWaitStream(
    int $_sequence_id,
  )[zoned_local]: Awaitable<HH\AsyncGenerator<null, string, void>> {
    $payloads = $this->streamPayloads;
    $encoder = ThriftStreamingSerializationHelpers::encodeStreamHelper(
      example_ExampleStreamingService_testStream_StreamResponse::class,
      new TCompactProtocolAccelerated(new TMemoryBuffer()),
    );
    return (
      async function()[zoned_local] use ($payloads, $encoder) {
        foreach ($payloads as $payload) {
          list($raw_bytes, $_is_app_ex) = $encoder($payload, null);
          yield $raw_bytes;
        }
      }
    )();
  }
}

/**
 * Handler that yields a StreamException as a stream payload to test
 * exception propagation through the decode path.
 */
<<Oncalls('thrift_hack')>>
final class TestStreamExceptionHandler extends TClientAsyncHandler {

  public function __construct(
    private TProtocol $recvProtocol,
    private TProtocol $sendProtocol,
    private example_ResponseStruct $firstResponse,
  ) {}

  <<__Override>>
  public async function genWaitStream(
    int $_sequence_id,
  )[zoned_local]: Awaitable<HH\AsyncGenerator<null, string, void>> {
    $transport = $this->sendProtocol->getTransport() as TMemoryBuffer;
    $method_name = '';
    $mtype = 0;
    $seqid = 0;
    $this->sendProtocol
      ->readMessageBegin(inout $method_name, inout $mtype, inout $seqid);
    $transport->resetBuffer();

    $first_response_struct =
      example_ExampleStreamingService_testStream_FirstResponse::fromShape(
        shape('success' => $this->firstResponse),
      );
    $response_bytes = TCompactSerializer::serialize($first_response_struct);
    $this->recvProtocol
      ->writeMessageBegin($method_name, TMessageType::REPLY, 0);
    $this->recvProtocol->getTransport()->write($response_bytes);
    $this->recvProtocol->writeMessageEnd();
    $this->recvProtocol->getTransport()->flush();

    // Encode a StreamException as a stream payload
    $encoder = ThriftStreamingSerializationHelpers::encodeStreamHelper(
      example_ExampleStreamingService_testStream_StreamResponse::class,
      new TCompactProtocolAccelerated(new TMemoryBuffer()),
    );
    $ex = new example_StreamException();
    $ex->message = 'stream error';

    return (
      async function()[zoned_local] use ($encoder, $ex) {
        list($raw_bytes, $_is_app_ex) = $encoder(null, $ex);
        yield $raw_bytes;
      }
    )();
  }
}

/**
 * Handler that tracks genAfterStream calls for testing.
 * Wraps a transport handler to provide stream I/O while recording chunks.
 */
<<Oncalls('thrift_hack')>>
final class TestStreamChunkTrackingHandler extends TClientAsyncHandler {

  private vec<string> $afterStreamChunks = vec[];
  private vec<string> $beforeStreamFuncNames = vec[];

  public function __construct(
    private TestStreamSinkAsyncHandler $transportHandler,
  ) {}

  <<__Override>>
  public async function genWaitStream(
    int $sequence_id,
  )[zoned_local]: Awaitable<HH\AsyncGenerator<null, string, void>> {
    return await $this->transportHandler->genWaitStream($sequence_id);
  }

  <<__Override>>
  public async function genBeforeStream(string $func_name): Awaitable<void> {
    $this->beforeStreamFuncNames[] = $func_name;
  }

  <<__Override>>
  public async function genAfterStream<<<__Explicit>> TResponse>(
    string $func_name,
    TResponse $response,
  )[zoned_local]: Awaitable<void> {
    if ($response is string) {
      $this->afterStreamChunks[] = $response;
    }
  }

  public function getAfterStreamChunks(): vec<string> {
    return $this->afterStreamChunks;
  }

  public function getBeforeStreamFuncNames(): vec<string> {
    return $this->beforeStreamFuncNames;
  }
}

<<Oncalls('thrift_hack')>>
final class TClientAsyncHandlerIntegrationTest extends WWWTest {

  // Stream: verifies first response and stream payloads round-trip
  public async function testStreamIntegration(): Awaitable<void> {
    $recv_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());
    $send_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());

    $handler = new TestStreamSinkAsyncHandler(
      $recv_protocol,
      $send_protocol,
      example_ResponseStruct::fromShape(shape('text' => 'hello')),
    );
    $handler->setStreamPayloads(vec['chunk1', 'chunk2', 'chunk3']);

    $client =
      new ExampleStreamingServiceAsyncClient($recv_protocol, $send_protocol);
    $client->setAsyncHandler($handler);

    $response_and_stream = await $client->testStream(
      example_RequestStruct::fromShape(shape('text' => 'req')),
    );

    expect($response_and_stream->response?->text)->toEqual('hello');

    $received = vec[];
    foreach ($response_and_stream->stream await as $chunk) {
      $received[] = $chunk;
    }
    expect($received)->toEqual(vec['chunk1', 'chunk2', 'chunk3']);
  }

  // Stream: verifies exception in stream payload is propagated
  public async function testStreamExceptionPropagation(): Awaitable<void> {
    $recv_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());
    $send_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());

    $handler = new TestStreamExceptionHandler(
      $recv_protocol,
      $send_protocol,
      example_ResponseStruct::fromShape(shape('text' => 'ok')),
    );

    $client =
      new ExampleStreamingServiceAsyncClient($recv_protocol, $send_protocol);
    $client->setAsyncHandler($handler);

    $response_and_stream = await $client->testStream(
      example_RequestStruct::fromShape(shape('text' => 'req')),
    );

    expect($response_and_stream->response?->text)->toEqual('ok');

    expect(async () ==> {
      foreach ($response_and_stream->stream await as $_chunk) {
        // Should throw on first iteration
      }
    })->toThrow(example_StreamException::class, 'stream error');
  }

  // Sink: verifies client payloads reach handler and final response returns
  public async function testSinkIntegration(): Awaitable<void> {
    $recv_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());
    $send_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());

    $handler = new TestStreamSinkAsyncHandler(
      $recv_protocol,
      $send_protocol,
      example_ResponseStruct::fromShape(shape('text' => 'sink-ack')),
    );
    $handler->setSinkProcessor(
      (vec<string> $payloads) ==> {
        return example_ResponseStruct::fromShape(shape(
          'text' => Str\join($payloads, ','),
        ));
      },
    );

    $client =
      new ExampleStreamingServiceAsyncClient($recv_protocol, $send_protocol);
    $client->setAsyncHandler($handler);

    $response_and_sink = await $client->testSink(
      example_RequestStruct::fromShape(shape('text' => 'req')),
    );

    expect($response_and_sink->response?->text)->toEqual('sink-ack');

    $payload_gen = (
      async function(): HH\AsyncGenerator<null, string, void> {
        yield 'a';
        yield 'b';
        yield 'c';
      }
    )();

    $sink_fn = $response_and_sink->genSink;
    $final_response = await $sink_fn($payload_gen);
    expect($final_response->text)->toEqual('a,b,c');
  }

  // Stream: verifies genAfterStream is called per chunk
  public async function testGenAfterStreamCalledPerChunk(): Awaitable<void> {
    MockJustKnobs::setBool('www/thrift:use_stream_async_handler_hooks', true);

    $recv_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());
    $send_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());

    $transport_handler = new TestStreamSinkAsyncHandler(
      $recv_protocol,
      $send_protocol,
      example_ResponseStruct::fromShape(shape('text' => 'hello')),
    );
    $transport_handler->setStreamPayloads(vec['chunk1', 'chunk2', 'chunk3']);

    $tracking_handler = new TestStreamChunkTrackingHandler($transport_handler);

    $client =
      new ExampleStreamingServiceAsyncClient($recv_protocol, $send_protocol);
    $client->setAsyncHandler($tracking_handler);

    $response_and_stream = await $client->testStream(
      example_RequestStruct::fromShape(shape('text' => 'req')),
    );

    expect($response_and_stream->response?->text)->toEqual('hello');

    // genAfterStream should not have been called yet (stream not consumed)
    expect($tracking_handler->getAfterStreamChunks())->toEqual(vec[]);

    $received = vec[];
    foreach ($response_and_stream->stream await as $chunk) {
      $received[] = $chunk;
    }

    expect($received)->toEqual(vec['chunk1', 'chunk2', 'chunk3']);
    expect($tracking_handler->getAfterStreamChunks())
      ->toEqual(vec['chunk1', 'chunk2', 'chunk3']);

    // genBeforeStream should have been called before each chunk + once for the
    // terminal next() that returns null (4 calls total for 3 chunks)
    expect(C\count($tracking_handler->getBeforeStreamFuncNames()))->toEqual(4);
  }

  // Multi-handler: genAfterStream is delegated to all handlers
  public async function testMultiHandlerGenAfterStreamDelegation(
  ): Awaitable<void> {
    MockJustKnobs::setBool('www/thrift:use_stream_async_handler_hooks', true);

    $recv_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());
    $send_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());

    $transport_handler = new TestStreamSinkAsyncHandler(
      $recv_protocol,
      $send_protocol,
      example_ResponseStruct::fromShape(shape('text' => 'multi')),
    );
    $transport_handler->setStreamPayloads(vec['a', 'b']);

    $tracking_handler = new TestStreamChunkTrackingHandler($transport_handler);

    $multi_handler = new TClientMultiAsyncHandler();
    $multi_handler->addHandler('transport', $tracking_handler);

    $client =
      new ExampleStreamingServiceAsyncClient($recv_protocol, $send_protocol);
    $client->setAsyncHandler($multi_handler);

    $response_and_stream = await $client->testStream(
      example_RequestStruct::fromShape(shape('text' => 'req')),
    );

    expect($response_and_stream->response?->text)->toEqual('multi');

    $received = vec[];
    foreach ($response_and_stream->stream await as $chunk) {
      $received[] = $chunk;
    }

    expect($received)->toEqual(vec['a', 'b']);
    expect($tracking_handler->getAfterStreamChunks())->toEqual(vec['a', 'b']);
  }

  // Multi-handler: stream payloads from multiple handlers are concatenated
  public async function testMultiHandlerConcatenatesStreams(): Awaitable<void> {
    $recv_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());
    $send_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());

    $handler1 = new TestStreamSinkAsyncHandler(
      $recv_protocol,
      $send_protocol,
      example_ResponseStruct::fromShape(shape('text' => 'multi')),
    );
    $handler1->setStreamPayloads(vec['a', 'b']);

    $handler2 = new TestStreamPayloadOnlyHandler(vec['c', 'd']);

    $multi_handler = new TClientMultiAsyncHandler();
    $multi_handler->addHandler('h1', $handler1);
    $multi_handler->addHandler('h2', $handler2);

    $client =
      new ExampleStreamingServiceAsyncClient($recv_protocol, $send_protocol);
    $client->setAsyncHandler($multi_handler);

    $response_and_stream = await $client->testStream(
      example_RequestStruct::fromShape(shape('text' => 'req')),
    );

    expect($response_and_stream->response?->text)->toEqual('multi');

    $received = vec[];
    foreach ($response_and_stream->stream await as $chunk) {
      $received[] = $chunk;
    }
    expect($received)->toEqual(vec['a', 'b', 'c', 'd']);
  }
}
