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
 * Tests for ThriftAsyncProcessor::getMethodMetadata() with service inheritance.
 *
 * Inheritance: ExampleChildService -> ExampleMiddleService -> ExampleService
 * Methods: sendChildRequest, sendMiddleRequest, sendRequest (respectively)
 *
 * Uses JKBoolDataProvider to run all tests with both old (process_) and new
 * (getMethodMetadata) code paths.
 */

<<
  Oncalls('thrift'),
  JKBoolDataProvider('thrift/hack:thrift_use_method_metadata_processor'),
>>
final class ThriftAsyncProcessorMethodMetadataTest extends WWWTest {

  public async function testExampleServiceOwnMethod(): Awaitable<void> {

    $handler = mock(ExampleRootServiceAsyncIf::class);
    $processor = new TestableExampleServiceAsyncProcessor($handler);

    $metadata = $processor->testGetMethodMetadata('sendRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      example_ExampleRootService_sendRequest_args::class,
    );
  }

  public async function testExampleMiddleServiceOwnMethod(): Awaitable<void> {
    $handler = mock(ExampleMiddleServiceAsyncIf::class);
    $processor = new TestableExampleMiddleServiceAsyncProcessor($handler);

    $metadata = $processor->testGetMethodMetadata('sendMiddleRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      example_ExampleMiddleService_sendMiddleRequest_args::class,
    );
  }

  public async function testExampleMiddleServiceInheritedMethod(
  ): Awaitable<void> {
    $handler = mock(ExampleMiddleServiceAsyncIf::class);
    $processor = new TestableExampleMiddleServiceAsyncProcessor($handler);

    // sendRequest is inherited from ExampleService via parent::getMethodMetadata()
    $metadata = $processor->testGetMethodMetadata('sendRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      example_ExampleRootService_sendRequest_args::class,
    );
  }

  public async function testExampleChildServiceOwnMethod(): Awaitable<void> {
    $handler = mock(ExampleChildServiceAsyncIf::class);
    $processor = new TestableExampleChildServiceAsyncProcessor($handler);

    $metadata = $processor->testGetMethodMetadata('sendChildRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      example_ExampleChildService_sendChildRequest_args::class,
    );
  }

  public async function testExampleChildServiceParentMethod(): Awaitable<void> {
    $handler = mock(ExampleChildServiceAsyncIf::class);
    $processor = new TestableExampleChildServiceAsyncProcessor($handler);

    // sendMiddleRequest is inherited from ExampleMiddleService
    $metadata = $processor->testGetMethodMetadata('sendMiddleRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      example_ExampleMiddleService_sendMiddleRequest_args::class,
    );
  }

  /**
   * KEY TEST: Verifies t_hack_generator.cc:5836-5849 delegation chain.
   *
   * ExampleChildService::getMethodMetadata('sendRequest')
   *   -> parent::getMethodMetadata (ExampleMiddleService)
   *     -> parent::getMethodMetadata (ExampleService)
   *       -> returns metadata
   */
  public async function testExampleChildServiceGrandparentMethod(
  ): Awaitable<void> {
    $handler = mock(ExampleChildServiceAsyncIf::class);
    $processor = new TestableExampleChildServiceAsyncProcessor($handler);

    // sendRequest is inherited from ExampleService (grandparent)
    $metadata = $processor->testGetMethodMetadata('sendRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      example_ExampleRootService_sendRequest_args::class,
    );
  }

  public async function testUnknownMethodReturnsNull(): Awaitable<void> {
    $handler = mock(ExampleChildServiceAsyncIf::class);
    $processor = new TestableExampleChildServiceAsyncProcessor($handler);

    $metadata = $processor->testGetMethodMetadata('unknownMethod');

    expect($metadata)->toBeNull();
  }
}

final class TestableExampleServiceAsyncProcessor
  extends ExampleRootServiceAsyncProcessor {

  public function __construct(ExampleRootServiceAsyncIf $handler) {
    parent::__construct($handler);
  }

  public function testGetMethodMetadata(
    string $fname,
  ): ?IThriftServiceMethodMetadata<ExampleRootServiceAsyncIf> {
    return static::getMethodMetadata($fname);
  }
}

final class TestableExampleMiddleServiceAsyncProcessor
  extends ExampleMiddleServiceAsyncProcessor {

  public function __construct(ExampleMiddleServiceAsyncIf $handler) {
    parent::__construct($handler);
  }

  public function testGetMethodMetadata(
    string $fname,
  ): ?IThriftServiceMethodMetadata<ExampleMiddleServiceAsyncIf> {
    return static::getMethodMetadata($fname);
  }
}

final class TestableExampleChildServiceAsyncProcessor
  extends ExampleChildServiceAsyncProcessor {

  public function __construct(ExampleChildServiceAsyncIf $handler) {
    parent::__construct($handler);
  }

  public function testGetMethodMetadata(
    string $fname,
  ): ?IThriftServiceMethodMetadata<ExampleChildServiceAsyncIf> {
    return static::getMethodMetadata($fname);
  }
}
