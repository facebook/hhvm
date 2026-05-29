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

<<Oncalls('thrift_hack')>>
final class ThriftEventHandlerMetadataTest extends WWWTest {
  public async function testMetadataBackwardCompatibility(): Awaitable<void> {
    $handler = mock(ExampleRootServiceAsyncIf::class);
    $proc = new ExampleRootServiceAsyncProcessor($handler);

    // The event handler will assert that the received metadata matches
    // the expected metadata provided here
    $event_handler =
      new VerifyNameEventHandler('example_ExampleRootService', 'doNothing');
    $proc->setEventHandler($event_handler);

    // Make the RPC call, which will invoke the event handler
    $input = new TCompactProtocol(new TMemoryBuffer());
    $input->writeRPCMessage(
      'doNothing',
      TMessageType::CALL,
      example_ExampleRootService_doNothing_args::withDefaultValues(),
      0,
    );
    $_ = await $proc->processAsync(
      $input,
      new TCompactProtocol(new TMemoryBuffer()),
      'doNothing',
    );
  }
}

final class VerifyNameEventHandler extends TProcessorEventHandler {
  public function __construct(
    private string $expectedServiceName,
    private string $expectedMethodName,
  ) {
  }

  <<__Override>>
  public function preExec(
    mixed $handler_context,
    string $service_name,
    string $method_name,
    mixed $args,
  ): void {
    expect($service_name)->toEqual($this->expectedServiceName);
    expect($method_name)->toEqual($this->expectedMethodName);
  }
}
