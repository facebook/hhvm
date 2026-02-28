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

/**
 * Tests for sink method event handler hooks in ThriftAsyncProcessor.
 *
 * Note: Currently there are NO sink-specific event hooks in TProcessorEventHandler.
 * Only basic hooks (preRead, postRead, preExec, postExec, preWrite, postWrite) are
 * called during sink processing. The streaming-specific hooks (postStreamStart,
 * postStreamPayloadGenerate, etc.) are NOT called for sink methods.
 *
 * See TProcessorEventHandler.php and ThriftProcessorBase::genExecuteSink for details.
 */
<<Oncalls('thrift')>>
final class ThriftProcessorSinkEventHandlerTest extends WWWTest {

  const string TEST_SINK_SERVICE_METHOD_NAME = 'testSink';

  private function mockServerSink(): TServerSink {
    $mock_server_sink = mock(TServerSink::class);
    $mock_server_sink->mockYield('genIsSinkReady', false);
    $mock_server_sink->mockReturn('getPayloads', tuple(vec[], null));
    $mock_server_sink->mockReturn('isClientException', false);
    $mock_server_sink->mockReturn('sendFinalResponse', null);
    return $mock_server_sink;
  }

  private function mockServerSinkWithClientException(): TServerSink {
    $mock_server_sink = mock(TServerSink::class);
    $mock_server_sink->mockYield('genIsSinkReady', false);
    $mock_server_sink->mockReturn('getPayloads', tuple(vec[], null));
    $mock_server_sink->mockReturn('isClientException', true);
    $mock_server_sink->mockReturn('sendFinalResponse', null);
    return $mock_server_sink;
  }

  private function mockSinkProtocols(
    TServerSink $server_sink,
  ): shape('input' => TProtocol, 'output' => TProtocol) {
    self::mockFunction(gen_start_thrift_sink<>)->mockYield($server_sink);

    self::mockFunction(PHP\hphp_set_error_page<>);
    self::mockFunction(php_root<>)->mockReturn('/var/www');
    self::mockFunction(request_stats_total_wall<>)->mockReturn(0);

    $mock_perf_metadata = mock(PerfMetadata::class);
    $mock_perf_metadata->mockReturn('setThriftFlush0Duration', null);
    self::mockFunction(PerfMetadata::get<>)->mockReturn($mock_perf_metadata);

    // Create input for the sink test
    $args =
      meta\thrift\example\ExampleStreamingService_testSink_args::withDefaultValues();
    $input_buffer = new TMemoryBuffer();
    $temp_protocol = new TBinaryProtocol($input_buffer);
    $args->write($temp_protocol);
    $temp_protocol->writeMessageEnd();

    // mock output protocol
    $output_transport = mock(TMemoryBuffer::class);
    $output_transport->mockReturn('getBuffer', '');
    $output_transport->mockReturn('resetBuffer', null);

    $input_protocol = new TBinaryProtocol($input_buffer);
    $output_protocol = new TBinaryProtocol($output_transport);

    return shape(
      'input' => $input_protocol,
      'output' => $output_protocol,
    );
  }

  /**
   * Test sink processAsync with both JK enabled and disabled.
   * Uses JKBoolDataProvider to run with both old (process_) and new (getMethodMetadata) paths.

   * Note: genExecuteSink does NOT call any event handler hooks because
   * TProcessorEventHandler has no sink-specific hooks defined.
   */
  <<JKBoolDataProvider('thrift/hack:thrift_use_method_metadata_processor')>>
  public async function testSinkProcessAsync(): Awaitable<void> {
    $mock_server_sink = $this->mockServerSink();
    $protocols = $this->mockSinkProtocols($mock_server_sink);
    $handler = new TestSinkThriftHandler();
    $processor = new TestSinkApplicationProcessor($handler);
    $event_handler = new TestSinkApplicationEventHandler();
    $processor->setEventHandler($event_handler);

    await $processor->processAsync(
      $protocols['input'],
      $protocols['output'],
      self::TEST_SINK_SERVICE_METHOD_NAME,
      1,
    );

    // Verify basic event hooks were called during sink processing
    expect($event_handler->preReadCalled)->toBeTrue();
    expect($event_handler->postReadCalled)->toBeTrue();
    expect($event_handler->preExecCalled)->toBeTrue();
    expect($event_handler->postExecCalled)->toBeTrue();
    expect($event_handler->preWriteCalled)->toBeTrue();
    expect($event_handler->postWriteCalled)->toBeTrue();
    expect($event_handler->requestName)->toBePHPEqual(
      self::TEST_SINK_SERVICE_METHOD_NAME,
    );
  }

  /**
   * Test sink processAsync with client exception.
   * Uses JKBoolDataProvider to run with both old and new code paths.
   *
   * When a client exception occurs during sink processing:
   * - Basic event hooks are still called (preRead, postRead, preExec, postExec, preWrite, postWrite)
   * - sendFinalResponse should NOT be called (handled by genExecuteSink)
   */
  <<JKBoolDataProvider('thrift/hack:thrift_use_method_metadata_processor')>>
  public async function testSinkProcessAsyncWithClientException(
  ): Awaitable<void> {
    $mock_server_sink = $this->mockServerSinkWithClientException();
    $protocols = $this->mockSinkProtocols($mock_server_sink);
    $handler = new TestSinkThriftHandler();
    $processor = new TestSinkApplicationProcessor($handler);
    $event_handler = new TestSinkApplicationEventHandler();
    $processor->setEventHandler($event_handler);

    await $processor->processAsync(
      $protocols['input'],
      $protocols['output'],
      self::TEST_SINK_SERVICE_METHOD_NAME,
      1,
    );

    // Verify basic event hooks were still called even with client exception
    expect($event_handler->preReadCalled)->toBeTrue();
    expect($event_handler->postReadCalled)->toBeTrue();
    expect($event_handler->preExecCalled)->toBeTrue();
    expect($event_handler->postExecCalled)->toBeTrue();
    expect($event_handler->preWriteCalled)->toBeTrue();
    expect($event_handler->postWriteCalled)->toBeTrue();
    expect($event_handler->requestName)->toBePHPEqual(
      self::TEST_SINK_SERVICE_METHOD_NAME,
    );
  }

  /**
   * Test sink processAsync when sink is cancelled by client.
   * Uses JKBoolDataProvider to run with both old and new code paths.
   *
   * When sink is cancelled by client (gen_start_thrift_sink returns null):
   * - Basic event hooks are still called (preRead, postRead, preExec, postExec, preWrite, postWrite)
   * - genExecuteSink returns early without error
   */
  <<JKBoolDataProvider('thrift/hack:thrift_use_method_metadata_processor')>>
  public async function testSinkProcessAsyncCancelledByClient(
  ): Awaitable<void> {
    // Mock gen_start_thrift_sink to return null (client cancelled)
    self::mockFunction(gen_start_thrift_sink<>)->mockYield(null);

    self::mockFunction(PHP\hphp_set_error_page<>);
    self::mockFunction(php_root<>)->mockReturn('/var/www');
    self::mockFunction(request_stats_total_wall<>)->mockReturn(0);

    $mock_perf_metadata = mock(PerfMetadata::class);
    $mock_perf_metadata->mockReturn('setThriftFlush0Duration', null);
    self::mockFunction(PerfMetadata::get<>)->mockReturn($mock_perf_metadata);

    // Create input for the sink test
    $args =
      meta\thrift\example\ExampleStreamingService_testSink_args::withDefaultValues();
    $input_buffer = new TMemoryBuffer();
    $temp_protocol = new TBinaryProtocol($input_buffer);
    $args->write($temp_protocol);
    $temp_protocol->writeMessageEnd();

    // mock output protocol
    $output_transport = mock(TMemoryBuffer::class);
    $output_transport->mockReturn('getBuffer', '');
    $output_transport->mockReturn('resetBuffer', null);

    $input_protocol = new TBinaryProtocol($input_buffer);
    $output_protocol = new TBinaryProtocol($output_transport);

    $handler = new TestSinkThriftHandler();
    $processor = new TestSinkApplicationProcessor($handler);
    $event_handler = new TestSinkApplicationEventHandler();
    $processor->setEventHandler($event_handler);

    // Should complete without error even when cancelled
    await $processor->processAsync(
      $input_protocol,
      $output_protocol,
      self::TEST_SINK_SERVICE_METHOD_NAME,
      1,
    );

    // Verify basic event hooks were still called even when sink was cancelled
    expect($event_handler->preReadCalled)->toBeTrue();
    expect($event_handler->postReadCalled)->toBeTrue();
    expect($event_handler->preExecCalled)->toBeTrue();
    expect($event_handler->postExecCalled)->toBeTrue();
    expect($event_handler->preWriteCalled)->toBeTrue();
    expect($event_handler->postWriteCalled)->toBeTrue();
    expect($event_handler->requestName)->toBePHPEqual(
      self::TEST_SINK_SERVICE_METHOD_NAME,
    );
  }
}

/**
 * Event handler for testing sink methods.
 * Only tracks basic hooks since TProcessorEventHandler has no sink-specific hooks.
 */
final class TestSinkApplicationEventHandler extends TProcessorEventHandler {
  public bool $preReadCalled = false;
  public bool $postReadCalled = false;
  public bool $preExecCalled = false;
  public bool $postExecCalled = false;
  public bool $preWriteCalled = false;
  public bool $postWriteCalled = false;
  public ?string $requestName = null;

  <<__Override>>
  public function preRead(
    mixed $handler_context,
    string $fn_name,
    mixed $args,
  )[write_props]: void {
    $this->preReadCalled = true;
    $this->requestName = $fn_name;
  }

  <<__Override>>
  public function postRead(
    mixed $handler_context,
    string $fn_name,
    mixed $args,
  )[write_props]: void {
    $this->postReadCalled = true;
  }

  <<__Override>>
  public function preExec(
    mixed $handler_context,
    string $service_name,
    string $fn_name,
    mixed $args,
  )[write_props]: void {
    $this->preExecCalled = true;
  }

  <<__Override>>
  public function postExec(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  )[write_props]: void {
    $this->postExecCalled = true;
  }

  <<__Override>>
  public function preWrite(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  )[write_props]: void {
    $this->preWriteCalled = true;
  }

  <<__Override>>
  public function postWrite(
    mixed $handler_context,
    string $fn_name,
    mixed $result,
  )[write_props]: void {
    $this->postWriteCalled = true;
  }
}

/**
 * Base interface for sink test handlers.
 */
class TestSinkServiceAsyncIf
  implements meta\thrift\example\ExampleStreamingServiceAsyncIf {

  public async function testStream(
    ?meta\thrift\example\RequestStruct $request,
  ): Awaitable<ResponseAndStream<meta\thrift\example\ResponseStruct, string>> {
    return new ResponseAndStream(
      meta\thrift\example\ResponseStruct::fromShape(shape('text' => 'test')),
      $this->genEmptyStream(),
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

  private async function genEmptyStream(
  ): HH\AsyncGenerator<null, string, void> {
    yield '';
  }
}

/**
 * Test handler for sink methods.
 */
final class TestSinkThriftHandler extends TestSinkServiceAsyncIf {

  <<__Override>>
  public async function testSink(
    ?meta\thrift\example\RequestStruct $_request,
  ): Awaitable<ResponseAndSink<
    meta\thrift\example\ResponseStruct,
    string,
    meta\thrift\example\ResponseStruct,
  >> {
    return new ResponseAndSink(
      meta\thrift\example\ResponseStruct::fromShape(
        shape('text' => 'sink-ack'),
      ),
      async (HH\AsyncGenerator<null, string, void> $_gen) ==> {
        return meta\thrift\example\ResponseStruct::fromShape(
          shape('text' => 'sink-final'),
        );
      },
    );
  }
}

/**
 * Test processor for sink methods.
 */
final class TestSinkApplicationProcessor
  extends meta\thrift\example\ExampleStreamingServiceAsyncProcessorBase {
  const type TThriftIf = TestSinkServiceAsyncIf;
}
