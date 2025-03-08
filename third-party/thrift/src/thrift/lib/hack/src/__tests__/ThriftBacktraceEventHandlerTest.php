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

<<Oncalls('xdc_artillery')>>
final class BacktraceClientEventHandlerTest extends WWWTest {
  use MethodLevelTest;

  public async function testPreRecvWithNullHeadersTransport(): Awaitable<void> {
    $handler = new BacktraceClientEventHandler(null);
    $fn_name = 'test_fn';
    $_ex_sequence_id = null;

    $handler->preRecv($fn_name, $_ex_sequence_id);
    expect(ArtilleryBacktrace::active())->toBeFalse();
  }

  public async function testPreRecvWithoutBacktraceHeader(): Awaitable<void> {
    $headers_transport = mock(TTransportSupportsHeaders::class)
      ->mockReturn('getReadHeaders', dict[]);
    $handler = new BacktraceClientEventHandler($headers_transport);
    $fn_name = 'test_fn';
    $_ex_sequence_id = null;

    $handler->preRecv($fn_name, $_ex_sequence_id);
    expect(ArtilleryBacktrace::active())->toBeFalse();
  }

  public async function testPreRecvWithInvalidBacktraceHeader(
  ): Awaitable<void> {
    $headers_transport = mock(TTransportSupportsHeaders::class)
      ->mockReturn('getReadHeaders', dict[
        Tracing_Backtrace\Backtrace_CONSTANTS::BACKTRACE_HEADER_KEY =>
          'invalid',
      ]);
    $handler = new BacktraceClientEventHandler($headers_transport);
    $fn_name = 'test_fn';
    $_ex_sequence_id = null;

    $handler->preRecv($fn_name, $_ex_sequence_id);
    expect(ArtilleryBacktrace::active())->toBeFalse();
  }

  public async function testPreRecvWithValidBacktraceHeader(): Awaitable<void> {
    $propagated_context = new Tracing_Backtrace\BacktracePropagatedContext();
    $propagated_context->requestId =
      ThriftContextPropState::generateRequestId();
    $propagated_context->downstreamBlockId = 1234;
    $propagated_context_str =
      TCompactSerializer::serialize($propagated_context);
    $encoded_propagated_context_str = Base64::encode($propagated_context_str);

    $headers_transport = mock(TTransportSupportsHeaders::class)
      ->mockReturn('getReadHeaders', dict[
        Tracing_Backtrace\Backtrace_CONSTANTS::BACKTRACE_HEADER_KEY =>
          $encoded_propagated_context_str,
      ]);
    $handler = new BacktraceClientEventHandler($headers_transport);
    $fn_name = 'test_fn';
    $_ex_sequence_id = null;

    self::mockFunction(ArtilleryBacktraceHelper::deserializeBacktracePayload<>)
      ->mockReturn($propagated_context);

    $handler->preRecv($fn_name, $_ex_sequence_id);
    expect(ArtilleryBacktrace::active())->toBeTrue();
  }

}

<<Oncalls('xdc_artillery')>>
final class ThriftBacktraceServerEventHandlerTest extends WWWTest {
  use ClassLevelTest;
  public async function testSetsHeaderWhenBacktraceActive(): Awaitable<void> {
    $thrift_server = mock(ThriftServer::class);
    $handler = new ThriftBacktraceServerEventHandler($thrift_server);

    $mock_block = mock(ArtilleryBlock::class);
    $mock_block->mockReturn('getID', 'AAAAAAA1234');

    ThriftContextPropState::get()->setRequestId(
      ThriftContextPropState::generateRequestId(),
    );

    self::mockFunction(ArtilleryBacktrace::getRequestBlock<>)->mockReturn(
      $mock_block,
    );

    $handler->preWrite(null, "", null);

    $expected_ctx = new Tracing_Backtrace\BacktracePropagatedContext();
    $expected_ctx->requestId = ThriftContextPropState::get()->getRequestId();
    $expected_ctx->downstreamBlockId = FBTraceEncode::decode('AAAAAAA1234');

    expect($thrift_server, 'addHTTPHeader')->wasCalledOnceWith(
      HTTPResponseHeader::ARTILLERY_BACKTRACE_HEADER,
      ArtilleryBacktraceHelper::serializeBacktracePayload($expected_ctx),
    );
  }
}
