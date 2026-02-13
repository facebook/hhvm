<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

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
  private ?(function(vec<string>): meta\thrift\example\ResponseStruct)
    $sinkProcessor = null;

  public function __construct(
    private TProtocol $recvProtocol,
    private TProtocol $sendProtocol,
    private meta\thrift\example\ResponseStruct $firstResponse,
  ) {}

  public function setStreamPayloads(vec<string> $payloads): void {
    $this->streamPayloads = $payloads;
  }

  public function setSinkProcessor(
    (function(vec<string>): meta\thrift\example\ResponseStruct) $processor,
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
      meta\thrift\example\ExampleStreamingService_testStream_FirstResponse::fromShape(
        shape('success' => $this->firstResponse),
      );
    $this->consumeRequestAndWriteFirstResponse(
      TCompactSerializer::serialize($first_response_struct),
    );

    $payloads = $this->streamPayloads ?? vec[];
    $encoder = ThriftStreamingSerializationHelpers::encodeStreamHelper(
      meta\thrift\example\ExampleStreamingService_testStream_StreamResponse::class,
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
      meta\thrift\example\ExampleStreamingService_testSink_FirstResponse::fromShape(
        shape('success' => $this->firstResponse),
      );
    $this->consumeRequestAndWriteFirstResponse(
      TCompactSerializer::serialize($first_response_struct),
    );

    $sink_processor = $this->sinkProcessor;
    $decoder = ThriftStreamingSerializationHelpers::decodeStreamHelper(
      meta\thrift\example\ExampleStreamingService_testSink_SinkPayload::class,
      'testSink',
      new TCompactProtocolAccelerated(new TMemoryBuffer()),
    );

    $encoder = ThriftStreamingSerializationHelpers::encodeStreamHelper(
      meta\thrift\example\ExampleStreamingService_testSink_FinalResponse::class,
      new TCompactProtocolAccelerated(new TMemoryBuffer()),
    );

    return async (HH\AsyncGenerator<null, string, void> $gen)[zoned_local] ==> {
      $received_payloads = vec[];
      foreach ($gen await as $raw_payload) {
        $received_payloads[] = $decoder($raw_payload, null);
      }
      $final_response = meta\thrift\example\ResponseStruct::fromShape(shape(
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
      meta\thrift\example\ExampleStreamingService_testStream_StreamResponse::class,
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
    private meta\thrift\example\ResponseStruct $firstResponse,
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
      meta\thrift\example\ExampleStreamingService_testStream_FirstResponse::fromShape(
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
      meta\thrift\example\ExampleStreamingService_testStream_StreamResponse::class,
      new TCompactProtocolAccelerated(new TMemoryBuffer()),
    );
    $ex = new meta\thrift\example\StreamException();
    $ex->message = 'stream error';

    return (
      async function()[zoned_local] use ($encoder, $ex) {
        list($raw_bytes, $_is_app_ex) = $encoder(null, $ex);
        yield $raw_bytes;
      }
    )();
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
      meta\thrift\example\ResponseStruct::fromShape(shape('text' => 'hello')),
    );
    $handler->setStreamPayloads(vec['chunk1', 'chunk2', 'chunk3']);

    $client = new meta\thrift\example\ExampleStreamingServiceAsyncClient(
      $recv_protocol,
      $send_protocol,
    );
    $client->setAsyncHandler($handler);

    $response_and_stream = await $client->testStream(
      meta\thrift\example\RequestStruct::fromShape(shape('text' => 'req')),
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
      meta\thrift\example\ResponseStruct::fromShape(shape('text' => 'ok')),
    );

    $client = new meta\thrift\example\ExampleStreamingServiceAsyncClient(
      $recv_protocol,
      $send_protocol,
    );
    $client->setAsyncHandler($handler);

    $response_and_stream = await $client->testStream(
      meta\thrift\example\RequestStruct::fromShape(shape('text' => 'req')),
    );

    expect($response_and_stream->response?->text)->toEqual('ok');

    expect(async () ==> {
      foreach ($response_and_stream->stream await as $_chunk) {
        // Should throw on first iteration
      }
    })->toThrow(meta\thrift\example\StreamException::class, 'stream error');
  }

  // Sink: verifies client payloads reach handler and final response returns
  public async function testSinkIntegration(): Awaitable<void> {
    $recv_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());
    $send_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());

    $handler = new TestStreamSinkAsyncHandler(
      $recv_protocol,
      $send_protocol,
      meta\thrift\example\ResponseStruct::fromShape(
        shape('text' => 'sink-ack'),
      ),
    );
    $handler->setSinkProcessor(
      (vec<string> $payloads) ==> {
        return meta\thrift\example\ResponseStruct::fromShape(shape(
          'text' => Str\join($payloads, ','),
        ));
      },
    );

    $client = new meta\thrift\example\ExampleStreamingServiceAsyncClient(
      $recv_protocol,
      $send_protocol,
    );
    $client->setAsyncHandler($handler);

    $response_and_sink = await $client->testSink(
      meta\thrift\example\RequestStruct::fromShape(shape('text' => 'req')),
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

  // Multi-handler: stream payloads from multiple handlers are concatenated
  public async function testMultiHandlerConcatenatesStreams(): Awaitable<void> {
    $recv_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());
    $send_protocol = new TCompactProtocolAccelerated(new TMemoryBuffer());

    $handler1 = new TestStreamSinkAsyncHandler(
      $recv_protocol,
      $send_protocol,
      meta\thrift\example\ResponseStruct::fromShape(shape('text' => 'multi')),
    );
    $handler1->setStreamPayloads(vec['a', 'b']);

    $handler2 = new TestStreamPayloadOnlyHandler(vec['c', 'd']);

    $multi_handler = new TClientMultiAsyncHandler();
    $multi_handler->addHandler('h1', $handler1);
    $multi_handler->addHandler('h2', $handler2);

    $client = new meta\thrift\example\ExampleStreamingServiceAsyncClient(
      $recv_protocol,
      $send_protocol,
    );
    $client->setAsyncHandler($multi_handler);

    $response_and_stream = await $client->testStream(
      meta\thrift\example\RequestStruct::fromShape(shape('text' => 'req')),
    );

    expect($response_and_stream->response?->text)->toEqual('multi');

    $received = vec[];
    foreach ($response_and_stream->stream await as $chunk) {
      $received[] = $chunk;
    }
    expect($received)->toEqual(vec['a', 'b', 'c', 'd']);
  }
}
