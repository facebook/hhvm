<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/**
 * Immutable version of ThriftContextPropState. Used in ContextPropV2
 * to pass an immutable version of ThriftContextPropState to
 * handlers that override the onIncomingUpstream or onOutgoingUpstream methods.
 */
final class ImmutableThriftContextPropState {
  private ThriftContextPropState $state;

  public function __construct(ThriftContextPropState $tcps) {
    $this->state = $tcps;
  }

  /**
  * Accessors
  */
  public readonly function getRequestId()[leak_safe]: string {
    return $this->state->getRequestId();
  }

  public readonly function getOriginId()[leak_safe]: ?int {
    return $this->state->getOriginId();
  }

  public readonly function getRegionalizationEntity()[leak_safe]: ?int {
    return $this->state->getRegionalizationEntity();
  }

  public function getTraceContextFlags1()[]: ?int {
    return $this->state->getTraceContextFlags1();
  }

  public function getBaggageFlags1()[]: ?int {
    return $this->state->getBaggageFlags1();
  }

  public function getBaggage()[]: ?ContextProp\Baggage {
    return $this->state->getBaggage();
  }

  public function getModelInfo()[]: ?ContextProp\ModelInfo {
    return $this->state->getModelInfo();
  }

  public function getModelTypeId()[]: ?int {
    return $this->state->getModelTypeId();
  }

  public function getUserIds()[]: ?ContextProp\UserIds {
    return $this->getBaggage()?->user_ids;
  }

  public function getFBUserId()[]: ?int {
    return $this->getUserIds()?->fb_user_id;
  }

  public function getIGUserId()[]: ?int {
    return $this->getUserIds()?->ig_user_id;
  }

  public function getTraceContext()[]: ?ContextProp\TraceContext {
    return $this->state->getTraceContext();
  }

  public function getExperimentIds()[]: vec<int> {
    return $this->state->getExperimentIds();
  }

  public function getPrivacyUniverse()[]: ?int {
    return $this->state->getPrivacyUniverseDesignator()?->getValue();
  }

  public function getRequestPriority()[]: ?RequestPriority {
    return $this->state->getRequestPriority();
  }

  /**
  * Returns true if any fields were set with non-empty values.
  */
  public function isSet()[write_props, zoned_shallow]: bool {
    return $this->state->isSet();
  }
}
