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
