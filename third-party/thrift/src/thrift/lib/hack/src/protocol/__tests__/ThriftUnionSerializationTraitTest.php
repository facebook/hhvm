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

<<Oncalls('thrift')>>
final class ThriftUnionSerializationTraitTest extends WWWTest {

  use ClassLevelTest;

  public static function provideUnionSerialize(
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

  <<DataProvider('provideUnionSerialize')>>
  public function testUnionSerialize(
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
}
