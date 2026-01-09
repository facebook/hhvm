<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

final class TraceSizeEstimationContextHandler implements IContextHandler {

  private readonly int $depth;
  private int $breadth;

  private static ?TraceSizeEstimationContextHandler $instance = null;

  private function __construct()[defaults] {
    $upstream_depth = ThriftContextPropState::get()->getDepth() ?? 0;
    $upstream_breadth = ThriftContextPropState::get()->getBreadth() ?? 0;

    $this->depth = $upstream_depth;
    $this->breadth = $upstream_breadth;
  }

  public static function getInstance(
  )[defaults]: TraceSizeEstimationContextHandler {
    if (self::$instance is null) {
      self::$instance = new self();
    }
    return self::$instance;
  }

  /**
   * Reset the singleton instance.
   */
  public static function resetInstance()[globals]: void {
    self::$instance = null;
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

    // write values
    $tracing_context->estimate->depth = readonly $this->depth + 1;
    $tracing_context->estimate->breadth = $this->breadth;

    // bump breadth for next service call
    $this->breadth++;
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
