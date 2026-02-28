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

  public function getAgentId()[]: ?string {
    return $this->getBaggage()?->agent_id;
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
