<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

final class TraceSizeEstimationContextHandler implements IContextHandler {

  private readonly int $upstreamDepth;

  private static ?TraceSizeEstimationContextHandler $instance = null;

  private function __construct()[defaults] {
    $upstream_depth = ThriftContextPropState::get()->getDepth() ?? 0;
    $this->upstreamDepth = $upstream_depth;
  }

  public static function getInstance(
  )[defaults]: TraceSizeEstimationContextHandler {
    if (self::$instance is null) {
      self::$instance = new self();
    }
    return self::$instance;
  }

  public function onIncomingDownstream(
    ThriftContextPropState $mutable_ctx,
    ClientInstrumentationParams $params,
    ImmutableThriftFrameworkMetadataOnResponse $immutable_tfmr,
  ): void {
  }

  public function onOutgoingDownstream(
    ClientInstrumentationParams $params,
    ThriftFrameworkMetadata $mutable_tfm,
    ImmutableThriftContextPropState $immutable_ctx,
  ): void {
    // only update propagate trace size values if tracing is on
    if (!ArtilleryInstrumentation::isTracingOn()) {
      return;
    }

    $mutable_tfm->baggage =
      $mutable_tfm->baggage ?? ContextProp\Baggage::withDefaultValues();
    $baggage = $mutable_tfm->baggage;
    $baggage->trace_context =
      $baggage->trace_context ?? ContextProp\TraceContext::withDefaultValues();
    $trace_context = $baggage->trace_context;
    $trace_context->tracing_context = $trace_context->tracing_context ??
      ContextProp\ArtilleryTracingContext::withDefaultValues();
    $tracing_context = $trace_context->tracing_context;
    $tracing_context->estimate = $tracing_context->estimate ??
      ContextProp\TraceSizeEstimation::withDefaultValues();

    // modify context values to send outbound
    $outgoing_depth = readonly $this->upstreamDepth + 1;
    $tracing_context->estimate->depth = $outgoing_depth;
  }

  public function onIncomingUpstream(
    ThriftContextPropState $mutable_ctx,
    ServerInstrumentationParams $params,
    ImmutableThriftFrameworkMetadata $immutable_tfm,
  ): void {}

  public function onOutgoingUpstream(
    ServerInstrumentationParams $params,
    ThriftFrameworkMetadataOnResponse $mutable_tfmr,
    ImmutableThriftContextPropState $immutable_ctx,
  ): void {}

}
