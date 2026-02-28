<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

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

    $handler = mock(meta\thrift\example\ExampleServiceAsyncIf::class);
    $processor = new TestableExampleServiceAsyncProcessor($handler);

    $metadata = $processor->testGetMethodMetadata('sendRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      meta\thrift\example\ExampleService_sendRequest_args::class,
    );
  }

  public async function testExampleMiddleServiceOwnMethod(): Awaitable<void> {
    $handler = mock(meta\thrift\example\ExampleMiddleServiceAsyncIf::class);
    $processor = new TestableExampleMiddleServiceAsyncProcessor($handler);

    $metadata = $processor->testGetMethodMetadata('sendMiddleRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      meta\thrift\example\ExampleMiddleService_sendMiddleRequest_args::class,
    );
  }

  public async function testExampleMiddleServiceInheritedMethod(
  ): Awaitable<void> {
    $handler = mock(meta\thrift\example\ExampleMiddleServiceAsyncIf::class);
    $processor = new TestableExampleMiddleServiceAsyncProcessor($handler);

    // sendRequest is inherited from ExampleService via parent::getMethodMetadata()
    $metadata = $processor->testGetMethodMetadata('sendRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      meta\thrift\example\ExampleService_sendRequest_args::class,
    );
  }

  public async function testExampleChildServiceOwnMethod(): Awaitable<void> {
    $handler = mock(meta\thrift\example\ExampleChildServiceAsyncIf::class);
    $processor = new TestableExampleChildServiceAsyncProcessor($handler);

    $metadata = $processor->testGetMethodMetadata('sendChildRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      meta\thrift\example\ExampleChildService_sendChildRequest_args::class,
    );
  }

  public async function testExampleChildServiceParentMethod(): Awaitable<void> {
    $handler = mock(meta\thrift\example\ExampleChildServiceAsyncIf::class);
    $processor = new TestableExampleChildServiceAsyncProcessor($handler);

    // sendMiddleRequest is inherited from ExampleMiddleService
    $metadata = $processor->testGetMethodMetadata('sendMiddleRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      meta\thrift\example\ExampleMiddleService_sendMiddleRequest_args::class,
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
    $handler = mock(meta\thrift\example\ExampleChildServiceAsyncIf::class);
    $processor = new TestableExampleChildServiceAsyncProcessor($handler);

    // sendRequest is inherited from ExampleService (grandparent)
    $metadata = $processor->testGetMethodMetadata('sendRequest');

    $metadata = expect($metadata)->toNotBeNull();
    expect($metadata)->toBeInstanceOf(
      ThriftServiceRequestResponseMethod::class,
    );
    $typed = $metadata as ThriftServiceRequestResponseMethod<_, _, _, _>;
    expect($typed->getArgsClass())->toEqual(
      meta\thrift\example\ExampleService_sendRequest_args::class,
    );
  }

  public async function testUnknownMethodReturnsNull(): Awaitable<void> {
    $handler = mock(meta\thrift\example\ExampleChildServiceAsyncIf::class);
    $processor = new TestableExampleChildServiceAsyncProcessor($handler);

    $metadata = $processor->testGetMethodMetadata('unknownMethod');

    expect($metadata)->toBeNull();
  }
}

final class TestableExampleServiceAsyncProcessor
  extends meta\thrift\example\ExampleServiceAsyncProcessor {

  public function __construct(
    meta\thrift\example\ExampleServiceAsyncIf $handler,
  ) {
    parent::__construct($handler);
  }

  public function testGetMethodMetadata(
    string $fname,
  ): ?IThriftServiceMethodMetadata<meta\thrift\example\ExampleServiceAsyncIf> {
    return static::getMethodMetadata($fname);
  }
}

final class TestableExampleMiddleServiceAsyncProcessor
  extends meta\thrift\example\ExampleMiddleServiceAsyncProcessor {

  public function __construct(
    meta\thrift\example\ExampleMiddleServiceAsyncIf $handler,
  ) {
    parent::__construct($handler);
  }

  public function testGetMethodMetadata(
    string $fname,
  ): ?IThriftServiceMethodMetadata<
    meta\thrift\example\ExampleMiddleServiceAsyncIf,
  > {
    return static::getMethodMetadata($fname);
  }
}

final class TestableExampleChildServiceAsyncProcessor
  extends meta\thrift\example\ExampleChildServiceAsyncProcessor {

  public function __construct(
    meta\thrift\example\ExampleChildServiceAsyncIf $handler,
  ) {
    parent::__construct($handler);
  }

  public function testGetMethodMetadata(
    string $fname,
  ): ?IThriftServiceMethodMetadata<
    meta\thrift\example\ExampleChildServiceAsyncIf,
  > {
    return static::getMethodMetadata($fname);
  }
}
