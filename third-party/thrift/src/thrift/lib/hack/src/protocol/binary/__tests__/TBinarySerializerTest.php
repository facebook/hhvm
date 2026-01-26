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
 * Test for class TBinarySerializer. This will check that
 * the serialization / deserialization actually works.
 */

<<Oncalls('thrift')>>
final class TBinarySerializerTest extends WWWTest {

  use ClassLevelTest;

  // Verifies if two objects of type CountersInformation
  // have the same content (counters value).
  private function checkEqualCounters(
    fb303\CountersInformation $obj1,
    fb303\CountersInformation $obj2,
    string $message,
  ): void {
    expect(C\count($obj2->data))->toBePHPEqual(
      C\count($obj1->data),
      '%s',
      $message,
    );
    foreach ($obj1->data as $string => $value) {
      expect(isset($obj2->data[$string]))->toBeTrue('%s', $message);
      expect($obj2->data[$string])->toBePHPEqual($value, '%s', $message);
    }
  }

  // Serializes a thrift object using normal write call on protocol.
  private function normalSerialize(fb303\CountersInformation $object): string {
    $transport = new TMemoryBuffer();
    $protocol = new TBinaryProtocolAccelerated($transport);
    $object->write($protocol);
    return $transport->getBuffer();
  }

  // Deserializes an object of specified class using normal read
  // call on protocol.
  private function normalDeserialize(
    string $object_string,
    classname<IThriftStruct> $class_name,
  ): IThriftStruct {
    $transport = new TMemoryBuffer($object_string);
    $protocol = new TBinaryProtocolAccelerated($transport);
    $object = new $class_name();
    $object->read($protocol);
    return $object;
  }

  // Tests serialization and deserialization methods
  // of TBinarySerializer.
  <<JKBoolDataProvider('thrift/hack:binary_struct')>>
  public function testSerializeAndDeserialize(): void {
    $obj1 = new fb303\CountersInformation();

    for ($i = 0; $i < 5; $i++) {
      $obj1->data['random_counter_'.$i] = $i;
    }

    // Both serialization methods should produce the same output.
    $obj1_string = $this->normalSerialize($obj1);
    $obj2_string = TBinarySerializer::serialize($obj1);

    expect($obj2_string)->toBePHPEqual(
      $obj1_string,
      '%s',
      'TBinarySerializer::serialize is broken',
    );

    // Ensure that our method used for validating against is not broken.
    $obj2 =
      $this->normalDeserialize($obj1_string, fb303\CountersInformation::class);
    $this->checkEqualCounters($obj1, $obj1, 'checkEqualCounters is broken');

    $this->checkEqualCounters(
      $obj1,
      $obj2 as fb303\CountersInformation,
      'Looks like our normalSerialize'.' / Deserialize is broken',
    );

    $obj3 = TBinarySerializer::deserialize(
      $obj2_string,
      new fb303\CountersInformation(),
    );
    $this->checkEqualCounters(
      $obj1,
      $obj3,
      'TBinarySerializer::deserialize_DEPRECATED is broken',
    );

  }

  <<JKBoolDataProvider('thrift/hack:binary_struct')>>
  public function testSerializeDeserializeComplexStruct(): void {
    $struct = CompactTestStruct::withDefaultValues();
    $struct->i1 = 42;
    $struct->b2 = true;
    $struct->doubles = vec[1.5, 2.5, 3.5];
    $struct->ints = dict[1 => true, 2 => true, 3 => true];
    $struct->m1 = dict['key1' => 10, 'key2' => 20];
    $struct->s = 'test string';

    $serialized = TBinarySerializer::serialize($struct);
    $deserialized = TBinarySerializer::deserialize(
      $serialized,
      CompactTestStruct::withDefaultValues(),
    );

    expect($deserialized->i1)->toEqual($struct->i1);
    expect($deserialized->b2)->toEqual($struct->b2);
    expect($deserialized->doubles)->toEqual($struct->doubles);
    expect($deserialized->ints)->toEqual($struct->ints);
    expect($deserialized->m1)->toEqual($struct->m1);
    expect($deserialized->s)->toEqual($struct->s);
  }
}
