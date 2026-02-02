<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

final class TraceSizeEstimationContextHandler implements IContextHandler {

  private readonly int $depth;
  private int $breadth;

  private static ?TraceSizeEstimationContextHandler $instance = null;

  /**
   * Cache for shouldBlockTraceContinuationOnOutgoingRequest result.
   * Once breadth exceeds the limit, all subsequent requests will also exceed it,
   * so we can cache the result to avoid redundant checks.
   */
  private static bool $shouldBlockTraceContinuation = false;

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
    self::$shouldBlockTraceContinuation = false;
  }

  /**
   * Get the current breadth value.
   */
  public function getBreadth()[]: int {
    return $this->breadth;
  }

  /**
   * Bump the breadth counter for non-Thrift service calls (TAO, Memcache).
   * This ensures breadth is properly tracked across all service types.
   */
  public function bumpBreadth()[write_props]: void {
    $this->breadth++;
  }

  private static function bumpBreadthRateLimitedCounter(): void {
    CategorizedOBC::typedGet(ODSCategoryID::ODS_ARTILLERY)->bumpEntityKey(
      'artillery_trace_continuation',
      'breadth_rate_limited',
    );
  }

  /**
   * Determines if trace continuation should be blocked on outgoing requests
   * based on the current breadth exceeding the max allowed breadth.
   *
   */
  public static function shouldBlockTraceContinuationOnOutgoingRequest(): bool {
    // If we've already determined trace should be blocked, bump ODS counter and return cached result
    if (self::$shouldBlockTraceContinuation) {
      self::bumpBreadthRateLimitedCounter();
      return true;
    }

    $breadth = self::getInstance()->getBreadth();
    $max_breadth = JustKnobs::getInt(
      'artillery/sdk_www:max_breadth_allowed_for_trace_continuation',
    );
    if ($max_breadth != 0 && $breadth > $max_breadth) {
      self::bumpBreadthRateLimitedCounter();
      // Only log to request block on first detection and if requested
      $block_props = new BlockProperties();
      $block_props->recordUserAnnotation(
        'serviceCallSuppressed',
        (string)$breadth,
      );
      ArtilleryInstrumentation::logToRequestBlock($block_props);
      // Cache the result so subsequent calls skip JK lookup and breadth check
      self::$shouldBlockTraceContinuation = true;
      return true;
    }
    return false;
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
