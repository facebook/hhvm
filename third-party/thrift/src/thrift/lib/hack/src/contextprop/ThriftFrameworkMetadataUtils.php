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

abstract final class ThriftFrameworkMetadataUtils {

  const string EXPERIMENT_IDS_IN_INCOMING_RESPONSE_HEADER_KEY =
    "experiment_ids_in_incoming_response";

  public static function decodeAndJoinExperimentIds(
    KeyedContainer<string, ?string> $propagated_response_headers,
  ): ?string {
    $tfmr_header_key =
      ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey;
    $tfmr_header = idx($propagated_response_headers, $tfmr_header_key);
    if ($tfmr_header is null) {
      return null;
    }

    $tfmr = ThriftFrameworkMetadataUtils::decodeFrameworkMetadataOnResponse(
      $tfmr_header,
    );
    if ($tfmr->experiment_ids is null) {
      return null;
    }

    $experiment_ids = $tfmr->experiment_ids->get_merge() ?? vec[];
    return Str\join($experiment_ids, ',');
  }

  <<__Memoize(#MakeICInaccessible)>>
  public static function decodeFrameworkMetadataOnResponse(
    string $encoded_response_tfm,
  ): ThriftFrameworkMetadataOnResponse {
    $tfmr = ThriftFrameworkMetadataOnResponse::withDefaultValues();
    $tfmr->read(
      new TCompactProtocolAccelerated(
        new TMemoryBuffer(Base64::decode($encoded_response_tfm)),
      ),
    );
    return $tfmr;
  }

  <<__Memoize(#KeyedByIC)>>
  public static function encodeThriftFrameworkMetadata(
    ThriftFrameworkMetadata $tfm,
  )[globals, zoned_shallow]: string {
    $buf = new TMemoryBuffer();
    $proto = new TCompactProtocolAccelerated($buf);
    $tfm->write($proto);
    return Base64::encode($buf->getBuffer());
  }

}
