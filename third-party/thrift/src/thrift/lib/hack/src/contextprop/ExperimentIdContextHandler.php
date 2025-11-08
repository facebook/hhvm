<?hh
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
 *
 */

final class ExperimentIdContextHandler implements IContextHandler {

  public function onIncomingDownstream(
    ThriftContextPropState $mutable_ctx,
    ClientInstrumentationParams $params,
    ImmutableThriftFrameworkMetadataOnResponse $immutable_tfmr,
  ): void {
    $ids_from_response = $immutable_tfmr->getExperimentIds()?->get_merge();
    if (SV_ZACHZUNDEL_KILLSWITCHES::experimentIdLogging()) {
      $length = C\count($ids_from_response ?? vec[]);
      CategorizedOBC::typedGet(ODSCategoryID::ODS_WEB_FOUNDATION)->bumpKey(
        'experiment_ids',
        $length,
        OdsAggregationType::ODS_AGGREGATION_TYPE_AVG,
      );

      CategorizedOBC::typedGet(ODSCategoryID::ODS_WEB_FOUNDATION)->bumpKey(
        'experiment_ids',
        $length,
      );
    }
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
