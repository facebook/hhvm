<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

final class ExperimentIdContextHandler implements IContextHandler {

  public function onIncomingDownstream(
    ThriftContextPropState $mutable_ctx,
    ClientInstrumentationParams $params,
    ImmutableThriftFrameworkMetadataOnResponse $immutable_tfmr,
  ): void {
    $ids_from_response = $immutable_tfmr->getExperimentIds()?->get_merge();
    if ($ids_from_response is nonnull && !C\is_empty($ids_from_response)) {
      foreach ($ids_from_response as $response_id) {
        if (!C\contains($mutable_ctx->getExperimentIds(), $response_id)) {
          $mutable_ctx->addExperimentId($response_id);
        }
      }
    }
  }

  public function onOutgoingDownstream(
    ClientInstrumentationParams $params,
    ThriftFrameworkMetadata $mutable_tfm,
    ImmutableThriftContextPropState $immutable_ctx,
  ): void {}

  public function onIncomingUpstream(
    ThriftContextPropState $mutable_ctx,
    ServerInstrumentationParams $params,
    ImmutableThriftFrameworkMetadata $immutable_tfm,
  ): void {}

  public function onOutgoingUpstream(
    ServerInstrumentationParams $params,
    ThriftFrameworkMetadataOnResponse $mutable_tfmr,
    ImmutableThriftContextPropState $immutable_ctx,
  ): void {
    $experiment_ids = $immutable_ctx->getExperimentIds();
    if (C\is_empty($experiment_ids)) {
      return;
    }
    $mutable_tfmr->experiment_ids = ExperimentIdsUpdate::fromShape(shape(
      'merge' => $experiment_ids,
    ));
  }
}
