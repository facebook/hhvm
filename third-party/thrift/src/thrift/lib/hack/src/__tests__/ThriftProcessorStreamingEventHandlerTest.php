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

<<Oncalls('thrift')>>
final class ThriftProcessorStreamingEventHandlerTest extends WWWTest {

  const string TEST_STREAMING_SERVICE_NAME = 'TestStreamingService';
  const string TEST_STREAMING_SERVICE_METHOD_NAME = 'testStream';

  private function mockServerStream(): TServerStream {
    $mock_server_stream = mock(TServerStream::class);
    $mock_server_stream->mockYield('genIsStreamReady', true);
    $mock_server_stream->mockYield('genSendStreamPayload', true);
    $mock_server_stream->mockReturn('sendStreamComplete', null);
    return $mock_server_stream;
  }

  private function mockProtocols(
    TServerStream $server_stream,
  ): shape('input' => TProtocol, 'output' => TProtocol) {
    self::mockFunction(PHP\gen_start_thrift_stream<>)
      ->mockYield($server_stream);

    self::mockFunction(PHP\hphp_set_error_page<>);
    self::mockFunction(php_root<>)->mockReturn('/var/www');
    self::mockFunction(request_stats_total_wall<>)->mockReturn(0);

    $mock_perf_metadata = mock(PerfMetadata::class);
    $mock_perf_metadata->mockReturn('setThriftFlush0Duration', null);
    self::mockFunction(PerfMetadata::get<>)->mockReturn($mock_perf_metadata);

    // Create input for the test
    $args =
      meta\thrift\example\ExampleStreamingService_testStream_args::withDefaultValues();
    $input_buffer = new \TMemoryBuffer();
    $temp_protocol = new \TBinaryProtocol($input_buffer);
    $args->write($temp_protocol);
    $temp_protocol->writeMessageEnd();

    // mock output protocol
    $output_transport = mock(TMemoryBuffer::class);
    $output_transport->mockReturn('getBuffer', '');
    $output_transport->mockReturn('resetBuffer', null);

    $input_protocol = new \TBinaryProtocol($input_buffer);
    $output_protocol = new \TBinaryProtocol($output_transport);

    return shape(
      'input' => $input_protocol,
      'output' => $output_protocol,
    );
  }

  public async function testStreamingEventHandlerHooks(): Awaitable<void> {
    $mock_server_stream = $this->mockServerStream();
    $protocols = $this->mockProtocols($mock_server_stream);
    $handler = new TestStreamingExampleThriftHandler();
    $processor = new TestStreamingApplicationProcessor($handler);
    $event_handler = new TestStreamingApplicationEventHandler();
    $processor->setEventHandler($event_handler);

    await $processor->processAsync(
      $protocols['input'],
      $protocols['output'],
      self::TEST_STREAMING_SERVICE_METHOD_NAME,
      1,
    );

    expect($event_handler->postStreamStartCalled)->toBeTrue();
    expect($event_handler->postStreamPayloadGenerateCalled)->toBeTrue();
    expect($event_handler->postStreamPayloadWriteCalled)->toBeTrue();
    expect($event_handler->requestName)->toBePHPEqual(
      self::TEST_STREAMING_SERVICE_METHOD_NAME,
    );
  }

  public async function testStreamingEventHandlerHooksWithUndeclaredException(
  ): Awaitable<void> {
    $mock_server_stream = $this->mockServerStream();
    $protocols = $this->mockProtocols($mock_server_stream);
    $handler = new TestStreamingErrorThriftHandler();
    $processor = new TestStreamingApplicationProcessor($handler);
    $event_handler = new TestStreamingApplicationEventHandler();
    $processor->setEventHandler($event_handler);

    await $processor->processAsync(
      $protocols['input'],
      $protocols['output'],
      self::TEST_STREAMING_SERVICE_METHOD_NAME,
      1,
    );

    expect($event_handler->postStreamStartCalled)->toBeTrue();
    expect($event_handler->postStreamPayloadGenerateCalled)->toBeTrue();
    expect($event_handler->postStreamPayloadErrorCalled)->toBeTrue();
    expect($event_handler->postStreamPayloadExceptionCalled)->toBeFalse();
  }

  public async function testStreamingEventHandlerHooksWithDefinedException(
  ): Awaitable<void> {
    $mock_server_stream = $this->mockServerStream();
    $protocols = $this->mockProtocols($mock_server_stream);
    $handler = new TestStreamingExceptionHandler();
    $processor = new TestStreamingApplicationProcessor($handler);
    $event_handler = new TestStreamingApplicationEventHandler();
    $processor->setEventHandler($event_handler);

    await $processor->processAsync(
      $protocols['input'],
      $protocols['output'],
      self::TEST_STREAMING_SERVICE_METHOD_NAME,
      1,
    );

    expect($event_handler->postStreamStartCalled)->toBeTrue();
    expect($event_handler->postStreamPayloadGenerateCalled)->toBeTrue();
    expect($event_handler->postStreamPayloadExceptionCalled)->toBeTrue();
    expect($event_handler->postStreamPayloadErrorCalled)->toBeFalse();
  }
}

final class TestStreamingApplicationEventHandler
  extends TProcessorEventHandler {
  public bool $postStreamStartCalled = false;
  public bool $postStreamPayloadGenerateCalled = false;
  public bool $postStreamPayloadWriteCalled = false;
  public bool $postStreamPayloadErrorCalled = false;
  public bool $postStreamPayloadExceptionCalled = false;
  public ?string $requestName = null;

  <<__Override>>
  public function postStreamStart(
    mixed $handler_context,
    string $fn_name,
  )[write_props]: void {
    $this->postStreamStartCalled = true;
    $this->requestName = $fn_name;
  }

  <<__Override>>
  public function postStreamPayloadGenerate(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  )[write_props]: void {
    $this->postStreamPayloadGenerateCalled = true;
    $this->requestName = $fn_name;
  }

  <<__Override>>
  public function postStreamPayloadWrite(
    mixed $handler_context,
    string $fn_name,
    mixed $payload,
  )[write_props]: void {
    $this->postStreamPayloadWriteCalled = true;
    $this->requestName = $fn_name;
  }

  <<__Override>>
  public function postStreamPayloadError(
    mixed $handler_context,
    string $fn_name,
    Exception $exception,
  )[write_props]: void {
    $this->postStreamPayloadErrorCalled = true;
    $this->requestName = $fn_name;
  }

  <<__Override>>
  public function postStreamPayloadException(
    mixed $handler_context,
    string $fn_name,
    Exception $exception,
  )[write_props]: void {
    $this->postStreamPayloadExceptionCalled = true;
    $this->requestName = $fn_name;
  }
}

class TestStreamingServiceAsyncIf
  implements \meta\thrift\example\ExampleStreamingServiceAsyncIf {

  public async function testStream(
    ?\meta\thrift\example\RequestStruct $request,
  ): Awaitable<ResponseAndStream<\meta\thrift\example\ResponseStruct, string>> {
    return new ResponseAndStream(
      meta\thrift\example\ResponseStruct::fromShape(shape('text' => 'test')),
      $this->emptyStream(),
    );
  }

  public async function testSink(
    ?meta\thrift\example\RequestStruct $_request,
  ): Awaitable<ResponseAndSink<
    meta\thrift\example\ResponseStruct,
    string,
    meta\thrift\example\ResponseStruct,
  >> {
    return new ResponseAndSink(
      meta\thrift\example\ResponseStruct::fromShape(shape('text' => 'test')),
      async (HH\AsyncGenerator<null, string, void> $_gen) ==> {
        return meta\thrift\example\ResponseStruct::fromShape(
          shape('text' => 'done'),
        );
      },
    );
  }

  private async function emptyStream(): HH\AsyncGenerator<null, string, void> {
    yield '';
  }
}

final class TestStreamingExampleThriftHandler
  extends TestStreamingServiceAsyncIf {

  <<__Override>>
  public async function testStream(
    ?\meta\thrift\example\RequestStruct $request,
  ): Awaitable<ResponseAndStream<\meta\thrift\example\ResponseStruct, string>> {
    return new ResponseAndStream(
      meta\thrift\example\ResponseStruct::fromShape(shape('text' => 'test')),
      $this->genStream(),
    );
  }

  private async function genStream(): HH\AsyncGenerator<null, string, void> {
    yield 'item1';
    yield 'item2';
    yield 'item3';
  }
}

final class TestStreamingErrorThriftHandler
  extends TestStreamingServiceAsyncIf {

  <<__Override>>
  public async function testStream(
    ?\meta\thrift\example\RequestStruct $request,
  ): Awaitable<ResponseAndStream<\meta\thrift\example\ResponseStruct, string>> {
    return new ResponseAndStream(
      meta\thrift\example\ResponseStruct::fromShape(shape('text' => 'test')),
      $this->genStreamWithError(),
    );
  }

  private async function genStreamWithError(
  ): HH\AsyncGenerator<null, string, void> {
    yield 'item1';
    throw new Exception('Undeclared stream error');
  }

}

final class TestStreamingExceptionHandler extends TestStreamingServiceAsyncIf {

  <<__Override>>
  public async function testStream(
    ?\meta\thrift\example\RequestStruct $request,
  ): Awaitable<ResponseAndStream<\meta\thrift\example\ResponseStruct, string>> {
    return new ResponseAndStream(
      meta\thrift\example\ResponseStruct::fromShape(shape('text' => 'test')),
      $this->genStreamWithException(),
    );
  }

  private async function genStreamWithException(
  ): HH\AsyncGenerator<null, string, void> {
    yield 'item1';
    $exception = meta\thrift\example\StreamException::fromShape(
      shape('message' => 'Declared stream exception'),
    );
    throw $exception;
  }
}

final class TestStreamingApplicationProcessor
  extends \meta\thrift\example\ExampleStreamingServiceAsyncProcessorBase {
  const type TThriftIf = TestStreamingServiceAsyncIf;
}
