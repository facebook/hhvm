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
final class ThriftContextPropServerEventHandlerTest extends WWWTest {
  use ClassLevelTest;
  public async function testThriftContextPropResponsePath(): Awaitable<void> {
    ThriftContextPropState::get()->addExperimentId(1);

    $encoded_response_tfm = "HBkWAgAA";

    $mock_thrift_server = mock(ThriftServer::class);
    $mock_thrift_server->mockImplementation('addHTTPHeader', function(
      $header,
      $value,
    ) use ($encoded_response_tfm) {
      expect($header)->toEqual(
        HTTPResponseHeader::THRIFT_FRAMEWORK_METADATA_RESPONSE,
      );
      expect($value)->toEqual($encoded_response_tfm);
    });
    $handler = new ThriftContextPropServerEventHandler($mock_thrift_server);
    $handler->preWrite(null, "", null);

    expect($mock_thrift_server, 'addHTTPHeader')->wasCalledOnceWith(
      HTTPResponseHeader::THRIFT_FRAMEWORK_METADATA_RESPONSE,
      $encoded_response_tfm,
    );
  }

  public async function testArtilleryContextPropResponsePath(
  ): Awaitable<void> {

    MockJustKnobs::setBool(
      'artillery/sdk_www:response_header_propagation_rollout',
      true,
    );
    // prevent the TFM from also being added to the header
    $_mock_thrift_context_prop_handler = self::mockFunction(
      ThriftContextPropHandler::makeResponseV<>,
    )->mockReturn(null);

    $mock_context_manager = mock(ContextManager::class);
    self::mockFunction(ContextManager::get<>)->mockReturn(
      $mock_context_manager,
    );
    $mock_context_manager->mockYield(
      'getCoreContext',
      (new CoreContextBuilder())->build(),
    );
    $mock_context_manager->mockImplementation(
      'processOutgoingResponse',
      function(shape() $_input) {
        return dict[
          ContextPropConstants_CONSTANTS::artillery_trace_ids_header =>
            '<test>',
        ];
      },
    );

    $encoded_trace_header = "<test>";
    $mock_thrift_server = mock(ThriftServer::class);
    $mock_thrift_server->mockImplementation('addHTTPHeader', function(
      $header,
      $value,
    ) use ($encoded_trace_header) {
      expect($header)->toEqual(HTTPResponseHeader::ARTILLERY_TRACING_HEADERS);
      expect($value)->toEqual($encoded_trace_header);
    });
    $handler = new ThriftContextPropServerEventHandler($mock_thrift_server);
    $handler->preWrite(null, "", null);

    expect($mock_thrift_server, 'addHTTPHeader')->wasCalledOnce();
  }
}
