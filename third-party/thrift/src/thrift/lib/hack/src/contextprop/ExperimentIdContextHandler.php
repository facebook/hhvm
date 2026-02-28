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
    $length = C\count($ids_from_response ?? vec[]);
    if (SV_ZACHZUNDEL_KILLSWITCHES::experimentIdLogging()) {
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

    $thrift_class = Shapes::idx($params, 'thrift_class');
    $limit = JustKnobs::getInt(
      'lumos/experimentation:www_experiment_downstream_limit',
      $thrift_class,
    );
    $current_num_experiments = C\count($mutable_ctx->getExperimentIds());
    if ($limit > 0 && $current_num_experiments >= $limit) {
      CategorizedOBC::typedGet(ODSCategoryID::ODS_FBTRACE)->bumpKey(
        'incoming_downstream_experiment_ids_dropped',
        1,
        OdsAggregationType::ODS_AGGREGATION_TYPE_COUNT,
      );
      CategorizedOBC::typedGet(ODSCategoryID::ODS_FBTRACE)->bumpKey(
        'incoming_downstream_experiment_ids_dropped',
        $length,
        OdsAggregationType::ODS_AGGREGATION_TYPE_SUM,
      );
      return;
    }

    if ($ids_from_response is nonnull && !C\is_empty($ids_from_response)) {
      foreach ($ids_from_response as $response_id) {
        if (!C\contains($mutable_ctx->getExperimentIds(), $response_id)) {
          $mutable_ctx->addExperimentId($response_id);
        }
      }
    }

    if (SV_ZACHZUNDEL_KILLSWITCHES::experimentIdLogging()) {
      $final_length = C\count($mutable_ctx->getExperimentIds());
      CategorizedOBC::typedGet(ODSCategoryID::ODS_FBTRACE)->bumpKey(
        'final_incoming_downstream_experiment_ids',
        $length,
        OdsAggregationType::ODS_AGGREGATION_TYPE_AVG,
      );

      CategorizedOBC::typedGet(ODSCategoryID::ODS_FBTRACE)->bumpKey(
        'final_incoming_downstream_experiment_ids',
        $length,
      );
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
