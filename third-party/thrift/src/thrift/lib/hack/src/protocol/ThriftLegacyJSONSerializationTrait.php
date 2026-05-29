<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
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
 */

/**
 * For Thrift strict union migration
 *
 * Behavior is different between `fb_json_serialize` and `JSONThriftSerializer`.
 * For unions that have been not yet been migrated to protected unions, we use
 * this trait to preserve existing serialization behavior.
 */
trait ThriftLegacyJSONSerializationTrait
  implements IThriftStruct, JsonSerializable {
  <<__Override>>
  public function jsonSerialize(): mixed {
    // we need to return a Map<string, mixed> as otherwise the string will be escaped again...
    // i.e. "{\"intData\":[1,2,3]}" instead of {"intData":[1,2,3]}
    $result = dict[];
    foreach ($this::SPEC as $_ => $field) {
      /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
      $field_value = $this->$field['var'];
      $result[$field['var']] = $field_value;
    }

    return $result;
  }
}
