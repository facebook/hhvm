<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('xdc_artillery')>>
final class TraceSizeEstimationContextHandlerTest extends WWWTest {

  use ClassLevelTest;

  <<__Override>>
  public async function beforeEach(): Awaitable<void> {
    ThriftContextPropState::get()->clear();
    TraceSizeEstimationContextHandler::resetInstance();
  }

  public async function testGetInstanceReturnsSingleton(): Awaitable<void> {
    $instance1 = TraceSizeEstimationContextHandler::getInstance();
    $instance2 = TraceSizeEstimationContextHandler::getInstance();

    expect($instance1 === $instance2)->toBeTrue();
  }

  public async function testGetInstanceAfterResetReturnsDifferentInstance(
  ): Awaitable<void> {
    $instance1 = TraceSizeEstimationContextHandler::getInstance();

    TraceSizeEstimationContextHandler::resetInstance();

    $instance2 = TraceSizeEstimationContextHandler::getInstance();

    expect($instance1 === $instance2)->toBeFalse();
  }

  public async function testOnOutgoingDownstreamDoesNothingWhenTracingOff(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(false);

    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    expect($mutable_tfm->baggage)->toBeNull();
  }

  public async function testOnOutgoingDownstreamPopulatesBaggageWhenTracingOn(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    expect($mutable_tfm->baggage)->toNotBeNull();
    expect($mutable_tfm->baggage?->trace_context)->toNotBeNull();
    expect($mutable_tfm->baggage?->trace_context?->tracing_context)
      ->toNotBeNull();
    expect($mutable_tfm->baggage?->trace_context?->tracing_context?->estimate)
      ->toNotBeNull();
  }

  public async function testOnOutgoingDownstreamSetsDepthToUpstreamPlusOne(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    ThriftContextPropState::get()->setDepth(5);

    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    $depth = $mutable_tfm->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->depth;
    expect($depth)->toEqual(6);
  }

  public async function testOnOutgoingDownstreamSetsDepthToOneWhenNoUpstreamDepth(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    $depth = $mutable_tfm->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->depth;
    expect($depth)->toEqual(1);
  }

  public async function testOnOutgoingDownstreamSetsBreadthFromUpstream(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    ThriftContextPropState::get()->setBreadth(10);

    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    $breadth = $mutable_tfm->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->breadth;
    expect($breadth)->toEqual(10);
  }

  public async function testOnOutgoingDownstreamIncrementsBreadthOnSubsequentCalls(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    ThriftContextPropState::get()->setBreadth(0);

    $handler = TraceSizeEstimationContextHandler::getInstance();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $mutable_tfm1 = ThriftFrameworkMetadata::withDefaultValues();
    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm1,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );
    $breadth1 = $mutable_tfm1->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->breadth;
    expect($breadth1)->toEqual(0);

    $mutable_tfm2 = ThriftFrameworkMetadata::withDefaultValues();
    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm2,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );
    $breadth2 = $mutable_tfm2->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->breadth;
    expect($breadth2)->toEqual(1);

    $mutable_tfm3 = ThriftFrameworkMetadata::withDefaultValues();
    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm3,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );
    $breadth3 = $mutable_tfm3->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->breadth;
    expect($breadth3)->toEqual(2);
  }

  public async function testOnOutgoingDownstreamMaintainsConsistentDepthAcrossMultipleCalls(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    ThriftContextPropState::get()->setDepth(3);

    $handler = TraceSizeEstimationContextHandler::getInstance();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $mutable_tfm1 = ThriftFrameworkMetadata::withDefaultValues();
    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm1,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );
    $depth1 = $mutable_tfm1->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->depth;
    expect($depth1)->toEqual(4);

    $mutable_tfm2 = ThriftFrameworkMetadata::withDefaultValues();
    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm2,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );
    $depth2 = $mutable_tfm2->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->depth;
    expect($depth2)->toEqual(4);

    $mutable_tfm3 = ThriftFrameworkMetadata::withDefaultValues();
    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm3,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );
    $depth3 = $mutable_tfm3->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->depth;
    expect($depth3)->toEqual(4);
  }

  public async function testOnOutgoingDownstreamPreservesExistingBaggageTraceContext(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $baggage = ContextProp\Baggage::withDefaultValues();
    $trace_context = ContextProp\TraceContext::withDefaultValues();
    $trace_context->flags1 = 123;
    $baggage->trace_context = $trace_context;
    $mutable_tfm->baggage = $baggage;

    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    expect($mutable_tfm->baggage?->trace_context?->flags1)->toEqual(123);
    expect(
      $mutable_tfm->baggage
        ?->trace_context
        ?->tracing_context
        ?->estimate
        ?->depth,
    )->toEqual(1);
  }

  public async function testOnOutgoingDownstreamHandlesLargeUpstreamValues(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    ThriftContextPropState::get()->setDepth(1000);
    ThriftContextPropState::get()->setBreadth(5000);

    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    $depth = $mutable_tfm->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->depth;
    $breadth = $mutable_tfm->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->breadth;

    expect($depth)->toEqual(1001);
    expect($breadth)->toEqual(5000);
  }

  public async function testOnIncomingDownstreamDoesNotModifyState(
  ): Awaitable<void> {
    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_ctx = ThriftContextPropState::get();
    $mutable_ctx->setDepth(10);
    $mutable_ctx->setBreadth(20);

    $tfmr = ThriftFrameworkMetadataOnResponse::withDefaultValues();
    $immutable_tfmr = new ImmutableThriftFrameworkMetadataOnResponse($tfmr);
    $params = shape();

    $handler->onIncomingDownstream($mutable_ctx, $params, $immutable_tfmr);

    expect($mutable_ctx->getDepth())->toEqual(10);
    expect($mutable_ctx->getBreadth())->toEqual(20);
  }

  public async function testOnIncomingUpstreamDoesNotModifyState(
  ): Awaitable<void> {
    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_ctx = ThriftContextPropState::get();
    $mutable_ctx->setDepth(10);
    $mutable_ctx->setBreadth(20);

    $tfm = ThriftFrameworkMetadata::withDefaultValues();
    $immutable_tfm = new ImmutableThriftFrameworkMetadata($tfm);
    $params = shape();

    $handler->onIncomingUpstream($mutable_ctx, $params, $immutable_tfm);

    expect($mutable_ctx->getDepth())->toEqual(10);
    expect($mutable_ctx->getBreadth())->toEqual(20);
  }

  public async function testOnOutgoingUpstreamDoesNotModifyTfmr(
  ): Awaitable<void> {
    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_tfmr = ThriftFrameworkMetadataOnResponse::withDefaultValues();
    $params = shape();

    $handler->onOutgoingUpstream(
      $params,
      $mutable_tfmr,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    expect($mutable_tfmr->experiment_ids)->toBeNull();
  }

  public async function testSingletonCapturesUpstreamValuesAtCreationTime(
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    ThriftContextPropState::get()->setDepth(5);
    ThriftContextPropState::get()->setBreadth(10);

    $handler = TraceSizeEstimationContextHandler::getInstance();

    ThriftContextPropState::get()->setDepth(100);
    ThriftContextPropState::get()->setBreadth(200);

    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    $depth = $mutable_tfm->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->depth;
    $breadth = $mutable_tfm->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->breadth;

    expect($depth)->toEqual(6);
    expect($breadth)->toEqual(10);
  }

  public static function dataProviderForDepthValues(
  ): dict<string, shape('upstream_depth' => int, 'expected_depth' => int)> {
    return dict[
      'depth 0' => shape('upstream_depth' => 0, 'expected_depth' => 1),
      'depth 1' => shape('upstream_depth' => 1, 'expected_depth' => 2),
      'depth 10' => shape('upstream_depth' => 10, 'expected_depth' => 11),
      'depth 100' => shape('upstream_depth' => 100, 'expected_depth' => 101),
    ];
  }

  <<DataProvider('dataProviderForDepthValues')>>
  public async function testOnOutgoingDownstreamIncreasesDepthByOne(
    int $upstream_depth,
    int $expected_depth,
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    ThriftContextPropState::get()->setDepth($upstream_depth);

    TraceSizeEstimationContextHandler::resetInstance();
    $handler = TraceSizeEstimationContextHandler::getInstance();
    $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $handler->onOutgoingDownstream(
      $params,
      $mutable_tfm,
      new ImmutableThriftContextPropState(ThriftContextPropState::get()),
    );

    $actual_depth = $mutable_tfm->baggage
      ?->trace_context
      ?->tracing_context
      ?->estimate
      ?->depth;
    expect($actual_depth)->toEqual($expected_depth);
  }

  public static function dataProviderForBreadthSequence(): dict<
    string,
    shape(
      'initial_breadth' => int,
      'num_calls' => int,
      'expected_breadth_sequence' => vec<int>,
    ),
  > {
    return dict[
      'start at 0 with 3 calls' => shape(
        'initial_breadth' => 0,
        'num_calls' => 3,
        'expected_breadth_sequence' => vec[0, 1, 2],
      ),
      'start at 5 with 3 calls' => shape(
        'initial_breadth' => 5,
        'num_calls' => 3,
        'expected_breadth_sequence' => vec[5, 6, 7],
      ),
      'start at 100 with 5 calls' => shape(
        'initial_breadth' => 100,
        'num_calls' => 5,
        'expected_breadth_sequence' => vec[100, 101, 102, 103, 104],
      ),
    ];
  }

  <<DataProvider('dataProviderForBreadthSequence')>>
  public async function testOnOutgoingDownstreamBreadthIncrementSequence(
    int $initial_breadth,
    int $num_calls,
    vec<int> $expected_breadth_sequence,
  ): Awaitable<void> {
    self::mockFunction(ArtilleryInstrumentation::isTracingOn<>)
      ->mockReturn(true);

    ThriftContextPropState::get()->setBreadth($initial_breadth);

    TraceSizeEstimationContextHandler::resetInstance();
    $handler = TraceSizeEstimationContextHandler::getInstance();
    $params = shape(
      'thrift_class' => ThriftClientBase::class,
      'client' =>
        new ThriftShimClient(new TBinaryProtocol(new TNullTransport())),
    );

    $actual_breadth_sequence = vec[];
    for ($i = 0; $i < $num_calls; $i++) {
      $mutable_tfm = ThriftFrameworkMetadata::withDefaultValues();
      $handler->onOutgoingDownstream(
        $params,
        $mutable_tfm,
        new ImmutableThriftContextPropState(ThriftContextPropState::get()),
      );
      $breadth = $mutable_tfm->baggage
        ?->trace_context
        ?->tracing_context
        ?->estimate
        ?->breadth;
      $actual_breadth_sequence[] = $breadth ?? -1;
    }

    expect($actual_breadth_sequence)->toEqual($expected_breadth_sequence);
  }
}
