<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

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
