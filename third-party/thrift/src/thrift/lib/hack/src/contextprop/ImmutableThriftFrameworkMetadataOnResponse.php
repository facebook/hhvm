<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/**
 * Immutable version of ThriftFrameworkMetadataOnResponse struct. Used in ContextPropV2
 * to pass an immutable version of ThriftFrameworkMetadataOnResponse to
 * handlers that override the onIncomingDownstream method.
 */
final class ImmutableThriftFrameworkMetadataOnResponse {
  private ?ExperimentIdsUpdate $experiment_ids;

  public function __construct(ThriftFrameworkMetadataOnResponse $tfmr) {
    $this->experiment_ids = $tfmr->experiment_ids;
  }

  public function getExperimentIds(): ?ExperimentIdsUpdate {
    return $this->experiment_ids;
  }

}
