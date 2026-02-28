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

// Reference implementations:
//  www/flib/core/smc/thrift/ThriftContextPropClientEventHandler.php
//  www/flib/core/smc/thrift/ThriftOutgoingContextPropHandler.php

final class ThriftContextPropHandler extends TClientEventHandler {

  public function __construct(
    private ?TTransportSupportsHeaders $headersTransport,
  )[] {}

  /**
   * Set the ThriftFrameworkMetadata header.
   */
  <<__Override>>
  public function preSend(
    string $fn_name,
    mixed $_args,
    int $sequence_id,
    string $_service_interface,
  )[globals, zoned_shallow]: void {
    $headers_transport = $this->headersTransport;
    if ($headers_transport is null) {
      return;
    }
    $v = self::makeV();
    if ($v !== null) {
      $headers_transport->setWriteHeader(
        ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey,
        $v,
      );
    }
  }

  /**
   * Create the value to be stored in the request TFM header:
   * Base64-encoded Compact-serialized ThriftFrameworkMetadata object.
   */
  public static function makeV()[zoned_local, globals]: ?string {
    $st = ThriftContextPropState::get();
    if (!$st->isSet()) {
      return null;
    }

    // Encapsulated logic: If the origin ID we obtain from the stack is null,
    // we'll just use the globally set one.
    return
      $st->getSerializedWithOriginIDOverride(TagManager::getLatestOriginID());
  }

  /**
   * Create the value to be stored in the response TFM header:
   * Base64-encoded Compact-serialized ThriftFrameworkMetadataOnResponse object.
   */
  public static function makeResponseV()[zoned_local, globals]: ?string {
    $tfm = ThriftContextPropState::get();
    if (!$tfm->isSet() || C\is_empty($tfm->getExperimentIds())) {
      return null;
    }

    $tfmr = ThriftFrameworkMetadataOnResponse::fromShape(
      shape(
        'experiment_ids' => ExperimentIdsUpdate::fromShape(shape(
          'merge' => $tfm->getExperimentIds(),
        )),
      ),
    );

    $buffer = TCompactSerializer::serialize($tfmr);
    $encoded = Base64::encode($buffer);
    return $encoded;
  }

  <<__Override>>
  public function preRecv(string $fn_name, ?int $_ex_sequence_id): void {
    if ($this->headersTransport is nonnull) {
      $encoded_tfmr = idx(
        $this->headersTransport->getReadHeaders(),
        ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey,
      );
      if ($encoded_tfmr is nonnull) {
        self::ingestFrameworkMetadataOnResponse($encoded_tfmr);
      }
    }
  }

  private static function ingestFrameworkMetadataOnResponse(
    string $encoded_response_tfm,
  ): void {
    $tfmr = ThriftFrameworkMetadataUtils::decodeFrameworkMetadataOnResponse(
      $encoded_response_tfm,
    );
    $ids_from_response = $tfmr->experiment_ids?->get_merge();
    if ($ids_from_response is nonnull && !C\is_empty($ids_from_response)) {
      $tfm = ThriftContextPropState::get();
      foreach ($ids_from_response as $response_id) {
        if (!C\contains($tfm->getExperimentIds(), $response_id)) {
          $tfm->addExperimentId($response_id);
        }
      }
    }
  }

}
