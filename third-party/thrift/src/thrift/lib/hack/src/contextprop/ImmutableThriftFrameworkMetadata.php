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
 * Immutable version of ThriftFrameworkMetadata. Used in ContextPropV2
 * to pass an immutable version of ThriftFrameworkMetadata to
 * handlers that override the onIncomingUpstream method.
 */
final class ImmutableThriftFrameworkMetadata {
  private ?string $request_id;
  private ?int $origin_id;
  private ?int $deadline_ms;
  private ?vec<int> $experiment_ids;
  private ?string $routingKey;
  private ?string $shardId;
  private ?string $loggingContext;
  private ?int $processingTimeout;
  private ?int $overallTimeout;
  private ?int $privacyUniverse;
  private ?RequestPriority $request_priority;
  private ?string $client_id;
  private ?\ContextProp\Baggage $baggage;

  public function __construct(ThriftFrameworkMetadata $tfm) {
    $this->request_id = $tfm->request_id;
    $this->origin_id = $tfm->origin_id;
    $this->deadline_ms = $tfm->deadline_ms;
    $this->experiment_ids = $tfm->experiment_ids;
    $this->routingKey = $tfm->routingKey;
    $this->shardId = $tfm->shardId;
    $this->loggingContext = $tfm->loggingContext;
    $this->processingTimeout = $tfm->processingTimeout;
    $this->overallTimeout = $tfm->overallTimeout;
    $this->privacyUniverse = $tfm->privacyUniverse;
    $this->request_priority = $tfm->request_priority;
    $this->client_id = $tfm->client_id;
    $this->baggage = $tfm->baggage;
  }

  public function getRequestId()[globals, zoned_shallow]: ?string {
    return $this->request_id;
  }

  public function getOriginId()[globals, zoned_shallow]: ?int {
    return $this->origin_id;
  }

  public function getDeadlineMs()[globals, zoned_shallow]: ?int {
    return $this->deadline_ms;
  }

  public function getExperimentIds()[globals, zoned_shallow]: ?vec<int> {
    return $this->experiment_ids;
  }

  public function getRoutingKey()[globals, zoned_shallow]: ?string {
    return $this->routingKey;
  }

  public function getShardId()[globals, zoned_shallow]: ?string {
    return $this->shardId;
  }

  public function getLoggingContext()[globals, zoned_shallow]: ?string {
    return $this->loggingContext;
  }

  public function getProcessingTimeout()[globals, zoned_shallow]: ?int {
    return $this->processingTimeout;
  }

  public function getOverallTimeout()[globals, zoned_shallow]: ?int {
    return $this->overallTimeout;
  }

  public function getPrivacyUniverse()[globals, zoned_shallow]: ?int {
    return $this->privacyUniverse;
  }

  public function getRequestPriority(
  )[globals, zoned_shallow]: ?RequestPriority {
    return $this->request_priority;
  }

  public function getClientId()[globals, zoned_shallow]: ?string {
    return $this->client_id;
  }

  public function getBaggage()[globals, zoned_shallow]: ?ContextProp\Baggage {
    return $this->baggage;
  }

}
