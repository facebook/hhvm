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

// Test helper: extends a thrift union with an extra property to verify
// __sleep preserves subclass fields during PHP serialization.
final class ThriftUnionSerializationTraitTestExtendedUnion
  extends SerializerTestUnion {
  public string $extraField = '';
}

<<Oncalls('thrift')>>
final class ThriftUnionSerializationTraitTest extends WWWTest {

  use ClassLevelTest;

  public static function provideUnionJsonSerialize(
  ): dict<string, (SerializerTestUnion, string, SerializerTestUnionEnum)> {
    return dict[
      'union_int' => tuple(
        SerializerTestUnion::fromShape(shape('int_value' => 12)),
        "{\"int_value\":12}",
        SerializerTestUnionEnum::int_value,
      ),
      'union_str' => tuple(
        SerializerTestUnion::fromShape(shape('str_value' => 'abc')),
        "{\"str_value\":\"abc\"}",
        SerializerTestUnionEnum::str_value,
      ),
      'union_list' => tuple(
        SerializerTestUnion::fromShape(shape('list_of_strings' => vec['abc'])),
        "{\"list_of_strings\":[\"abc\"]}",
        SerializerTestUnionEnum::list_of_strings,
      ),
      'union_set' => tuple(
        SerializerTestUnion::fromShape(
          shape('set_of_strings' => dict['abc' => false]),
        ),
        "{\"set_of_strings\":[\"abc\"]}",
        SerializerTestUnionEnum::set_of_strings,
      ),
      'union_map' => tuple(
        SerializerTestUnion::fromShape(
          shape('map_of_int_to_strings' => dict[12 => 'abc']),
        ),
        "{\"map_of_int_to_strings\":{\"12\":\"abc\"}}",
        SerializerTestUnionEnum::map_of_int_to_strings,
      ),
      'union_struct' => tuple(
        SerializerTestUnion::fromShape(shape(
          'test_struct' =>
            CompactTestStructSmall::fromShape(shape('ints' => vec[1, 2])),
        )),
        "{\"test_struct\":{\"ints\":[1,2]}}",
        SerializerTestUnionEnum::test_struct,
      ),
    ];
  }

  <<DataProvider('provideUnionJsonSerialize')>>
  public function testUnionJsonSerialize(
    SerializerTestUnion $object,
    string $serialized_string,
    SerializerTestUnionEnum $set_field,
  ): void {
    expect($serialized_string)->toBePHPEqual(
      JSONThriftSerializer::serialize($object),
    );
    $test_object = SerializerTestUnion::withDefaultValues();
    JSONThriftSerializer::deserialize($serialized_string, $test_object);
    expect($set_field)->toBePHPEqual($test_object->getType());
  }

  public static function provideUseHHVMExtension(): dict<string, (bool)> {
    return dict[
      'disable_hhvm_extension' => tuple(true),
      'enable_hhvm_extension' => tuple(false),
    ];
  }

  <<DataProvider('provideUseHHVMExtension')>>
  public function testDeserializeUnionWithMultipleFields(
    bool $disable_hphp_extension,
  ): void {
    $union = new SerializerTestUnion(1, "test");
    $serialized =
      TCompactSerializer::serialize($union, null, $disable_hphp_extension);

    // deserializing non-strict union with multiple fields is okay
    $deserialized = TCompactSerializer::deserialize($serialized, $union);
    expect($deserialized->get_int_value())->toBePHPEqual(
      $union->get_int_value(),
    );
    expect($deserialized->get_str_value())->toBePHPEqual(
      $union->get_str_value(),
    );

    // deserializing strict union with multiple fields should throw
    $strict_union = new SerializerTestStrictUnion();
    expect(
      () ==> TCompactSerializer::deserialize(
        $serialized,
        $strict_union,
        null,
        $disable_hphp_extension,
      ),
    )
      ->toThrow(TProtocolException::class, 'Union field already set');
  }

  public static function providePhpSerializeRoundTrip(): dict<string, shape(
    'union' => SerializerTestUnion,
    'expected_type' => SerializerTestUnionEnum,
  )> {
    return dict[
      'int_value' => shape(
        'union' => SerializerTestUnion::fromShape(shape('int_value' => 42)),
        'expected_type' => SerializerTestUnionEnum::int_value,
      ),
      'str_value' => shape(
        'union' =>
          SerializerTestUnion::fromShape(shape('str_value' => 'hello')),
        'expected_type' => SerializerTestUnionEnum::str_value,
      ),
      'list_of_strings' => shape(
        'union' => SerializerTestUnion::fromShape(
          shape('list_of_strings' => vec['a', 'b', 'c']),
        ),
        'expected_type' => SerializerTestUnionEnum::list_of_strings,
      ),
      'struct_value' => shape(
        'union' => SerializerTestUnion::fromShape(shape(
          'test_struct' =>
            CompactTestStructSmall::fromShape(shape('ints' => vec[1, 2, 3])),
        )),
        'expected_type' => SerializerTestUnionEnum::test_struct,
      ),
    ];
  }

  <<DataProvider('providePhpSerializeRoundTrip')>>
  public function testPhpSerializeDeserializeRoundTrip(
    SerializerTestUnion $union,
    SerializerTestUnionEnum $expected_type,
  ): void {
    // @lint-ignore DEPRECATED_FUNCTION15
    $serialized = PHP\serialize($union);
    // @lint-ignore DEPRECATED_FUNCTION16
    $deserialized = PHP\unserialize($serialized);

    expect($deserialized)->toBeInstanceOf(SerializerTestUnion::class);
    $deserialized as SerializerTestUnion;
    expect($deserialized->getType())->toEqual($expected_type);

    // Verify the value round-trips by re-serializing via Thrift
    expect(JSONThriftSerializer::serialize($deserialized))->toEqual(
      JSONThriftSerializer::serialize($union),
    );
  }

  public function testPhpSerializeDeserializeStrictUnion(): void {
    $union = SerializerTestStrictUnion::fromShape(shape('int_value' => 99));

    // @lint-ignore DEPRECATED_FUNCTION15
    $serialized = PHP\serialize($union);
    // @lint-ignore DEPRECATED_FUNCTION16
    $deserialized = PHP\unserialize($serialized);

    expect($deserialized)->toBeInstanceOf(SerializerTestStrictUnion::class);
    $deserialized as SerializerTestStrictUnion;
    expect($deserialized->getType())->toEqual(
      SerializerTestStrictUnionEnum::int_value,
    );
    expect($deserialized->get_int_value())->toEqual(99);
  }

  public function testPhpSerializeDeserializeEmptyUnion(): void {
    $union = SerializerTestUnion::withDefaultValues();

    // @lint-ignore DEPRECATED_FUNCTION15
    $serialized = PHP\serialize($union);
    // @lint-ignore DEPRECATED_FUNCTION16
    $deserialized = PHP\unserialize($serialized);

    expect($deserialized)->toBeInstanceOf(SerializerTestUnion::class);
    $deserialized as SerializerTestUnion;
    expect($deserialized->getType())->toEqual(SerializerTestUnionEnum::_EMPTY_);
  }

  public function testPhpSerializeDeserializeExtendedUnion(): void {
    $union = new ThriftUnionSerializationTraitTestExtendedUnion(42);
    $union->extraField = 'extra_data';

    // @lint-ignore DEPRECATED_FUNCTION15
    $serialized = PHP\serialize($union);
    // @lint-ignore DEPRECATED_FUNCTION16
    $deserialized = PHP\unserialize($serialized);

    expect($deserialized)->toBeInstanceOf(
      ThriftUnionSerializationTraitTestExtendedUnion::class,
    );
    $deserialized as ThriftUnionSerializationTraitTestExtendedUnion;
    expect($deserialized->get_int_value())->toEqual(42);
    expect($deserialized->extraField)->toEqual('extra_data');
  }
}
