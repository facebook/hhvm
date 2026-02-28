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
final class SerializationTest extends WWWTest {

  use ClassLevelTest;

  <<__LateInit>> private CompactTestStruct $testValue;
  <<__LateInit>> private string $compactSerialized;
  <<__LateInit>> private string $binarySerialized;
  <<__LateInit>> private string $jsonSerialized;
  <<__LateInit>> private string $compactSerializedNoExtension;
  <<__LateInit>> private string $binarySerializedNoExtension;

  const DELTA = 1e-12;

  <<__Override>>
  public async function beforeEach(): Awaitable<void> {
    $v = CompactTestStruct::withDefaultValues();
    $v->i1 = 5;
    $v->i2 = null;
    $v->b2 = true;
    $v->doubles = vec[1.2, 3.4, 12.345000267028809];
    $v->ints = dict[-1 => true, 0 => true, 1 => true];
    $v->m1 = dict['a' => 1, 'b' => 2];
    $v->m2 = dict[1 => vec['asdf', 'abc']];
    $v->structs =
      vec[CompactTestStructSmall::fromShape(shape('bools' => vec[true]))];
    $v->f = 12.345000267028809;
    $v->fmap = dict[4 => 1.25];
    $v->i16_test = 2;
    $v->i32_test = 3;
    $v->byte_test = 4;
    $v->s = 'xyz';
    $v->empty_map = dict[];
    $v->null_map = null;
    $v->map_to_map = dict[1 => dict[2 => "3"], 2 => dict[2 => ""]];

    $this->testValue = $v;

    // TODO ailic: change this to true after floats are pushed to thrift
    $this->compactSerialized = TCompactSerializer::serialize($v);
    $this->binarySerialized = TBinarySerializer::serialize($v);
    $this->jsonSerialized = JSONThriftSerializer::serialize($v);

    $this->compactSerializedNoExtension =
      TCompactSerializer::serialize($v, null, true);
    $this->binarySerializedNoExtension = TBinarySerializer::serialize($v, true);
  }

  public function testToFromShape(): void {
    expect(CompactTestStruct::__fromShape($this->testValue->__toShape()))
      ->toBePHPEqual($this->testValue);
  }

  public function testCompactSerialize(): void {
    expect($this->compactSerialized)->toEqual(
      $this->compactSerializedNoExtension,
    );
  }

  // Tests Compact, Binary, and JSON deserialization for both the HPHP extension
  // and Hack implementations.
  public function testDeserialize(): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      expect(TCompactSerializer::deserialize(
        $this->compactSerialized,
        CompactTestStruct::withDefaultValues(),
        null,
        $disable_hphp_extension,
      ))->toBePHPEqual($this->testValue);

      expect(TBinarySerializer::deserialize(
        $this->binarySerialized,
        CompactTestStruct::withDefaultValues(),
        $disable_hphp_extension,
      ))->toBePHPEqual($this->testValue);

      expect(TCompactSerializer::deserialize(
        $this->compactSerializedNoExtension,
        CompactTestStruct::withDefaultValues(),
        null,
        $disable_hphp_extension,
      ))
        ->toBePHPEqual($this->testValue);

      expect(TBinarySerializer::deserialize(
        $this->binarySerializedNoExtension,
        CompactTestStruct::withDefaultValues(),
        $disable_hphp_extension,
      ))->toBePHPEqual($this->testValue);
    }

    // JSON has delta in accuracy for floats.
    expect(JSONThriftSerializer::deserialize(
      $this->jsonSerialized,
      CompactTestStruct::withDefaultValues(),
    ))
      ->toEqualWithDelta($this->testValue, self::DELTA);
  }

  // verifies that calling $function causes no logging if $mentions is empty, or
  // 1 log message mentioning $mention otherwise.
  private function assertLogMentions(
    (function(): void) $function,
    ?string $mention,
  ): void {
    FBLogger::pushMessageClass(TestLogMessage::class);
    TestLogMessage::clear();
    $function();
    $msgs = TestLogMessage::getLogMessages();
    if ($mention !== null) {
      expect(C\count($msgs))->toBePHPEqual(1);
      expect(C\firstx($msgs)->getMessage())->toContainSubstring(
        'consume the whole input string',
      );
    } else {
      expect($msgs)->toBeEmpty();
    }
    FBLogger::popMessageClass();
  }

  // Deserialization works happily with just the right about of data
  public function testGoodCompact(): void {
    $this->assertLogMentions(
      function() {
        $result = TCompactSerializer::deserialize(
          $this->compactSerialized,
          CompactTestStruct::withDefaultValues(),
        );
        expect($result)->toBePHPEqual($this->testValue);
      },
      null,
    );
    $this->assertLogMentions(
      function() {
        $result = TCompactSerializer::deserialize_DEPRECATED(
          $this->compactSerializedNoExtension,
          'CompactTestStruct',
          null,
          true,
        );
        expect($result)->toBePHPEqual($this->testValue);
      },
      null,
    );
  }

  // Deserialization still works with extra data, but logs
  public function testBadCompact(): void {
    $this->assertLogMentions(
      function() {
        $result = TCompactSerializer::deserialize(
          $this->compactSerialized.'extra!',
          CompactTestStruct::withDefaultValues(),
        );
        expect($result)->toBePHPEqual($this->testValue);
      },
      null,
    );
    $this->assertLogMentions(
      function() {
        $result = TCompactSerializer::deserialize_DEPRECATED(
          $this->compactSerializedNoExtension.'extra!',
          'CompactTestStruct',
          null,
          true,
        );
        expect($result)->toBePHPEqual($this->testValue);
      },
      null,
    );
  }

  // Deserialization works happily with just the right about of data
  public function testGoodBinary(): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $this->assertLogMentions(
        () ==> {
          $result = TBinarySerializer::deserialize(
            $this->binarySerialized,
            CompactTestStruct::withDefaultValues(),
            $disable_hphp_extension,
          );
          expect($result)->toBePHPEqual($this->testValue);
        },
        null,
      );
      $this->assertLogMentions(
        () ==> {
          $result = TBinarySerializer::deserialize_DEPRECATED(
            $this->binarySerializedNoExtension,
            CompactTestStruct::class,
            $disable_hphp_extension,
          );
          expect($result)->toBePHPEqual($this->testValue);
        },
        null,
      );

    }
  }

  // Deserialization still works with extra data, but logs
  public function testBadBinary(): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $this->assertLogMentions(
        () ==> {
          $result = TBinarySerializer::deserialize(
            $this->binarySerialized.'extra!',
            CompactTestStruct::withDefaultValues(),
            $disable_hphp_extension,
          );
          expect($result)->toBePHPEqual($this->testValue);
        },
        null,
      );
      $this->assertLogMentions(
        () ==> {
          $result = TBinarySerializer::deserialize_DEPRECATED(
            $this->binarySerializedNoExtension.'extra!',
            CompactTestStruct::class,
            $disable_hphp_extension,
          );
          expect($result)->toBePHPEqual($this->testValue);
        },
        null,
      );
    }
  }

  public function testGoodJSON(): void {
    $result = JSONThriftSerializer::deserialize(
      $this->jsonSerialized,
      CompactTestStruct::withDefaultValues(),
    );
    expect($result)->toEqualWithDelta($this->testValue, self::DELTA);
  }

  <<ExpectedException(TProtocolException::class)>>
  public function testBadJSON(): void {
    JSONThriftSerializer::deserialize(
      'not_really_json',
      CompactTestStruct::withDefaultValues(),
    );
  }

  public function testJSONArray(): void {
    $v = CompactTestStruct::withDefaultValues();
    $v->i1 = 6;
    $v->b2 = true;
    $v->doubles = vec[10.2, 30.4];

    $objects = Vector {$this->testValue, $v};
    $result = JSONThriftSerializer::serializeList($objects);
    $result_objects = JSONThriftSerializer::deserializeList(
      $result,
      CompactTestStruct::withDefaultValues(),
    );
    expect(C\count($result_objects))->toBePHPEqual(2);
    expect($result_objects)->toEqualWithDelta($objects, self::DELTA);

    $objects = Vector {$this->testValue};
    $result = JSONThriftSerializer::serializeList($objects);
    $result_objects = JSONThriftSerializer::deserializeList(
      $result,
      CompactTestStruct::withDefaultValues(),
    );
    expect(C\count($result_objects))->toBePHPEqual(1);
    expect($result_objects)->toEqualWithDelta($objects, self::DELTA);

    $objects = Vector {};
    $result = JSONThriftSerializer::serializeList($objects);
    $result_objects = JSONThriftSerializer::deserializeList(
      $result,
      CompactTestStruct::withDefaultValues(),
    );
    expect(C\count($result_objects))->toBePHPEqual(0);
    expect($result_objects)->toBePHPEqual($objects);
  }

  private function assertRoundTripI16(int $value): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $s = CompactTestStruct::withDefaultValues();
      $s->i16_test = $value;
      $serialized =
        TCompactSerializer::serialize($s, null, $disable_hphp_extension);
      $result = TCompactSerializer::deserialize(
        $serialized,
        CompactTestStruct::withDefaultValues(),
        null,
        $disable_hphp_extension,
      );
      expect($value)->toBePHPEqual($result->i16_test);
    }
  }

  private function assertThrowsOutOfRangeI16(int $value): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $s = CompactTestStruct::withDefaultValues();
      $s->i16_test = $value;
      expect(() ==> {
        TCompactSerializer::serialize($s, null, $disable_hphp_extension);
      })->toThrow(TProtocolException::class, null, 'Value is out of range');
    }
  }

  public function testI16Range(): void {
    $i16_min = -(1 << 15);
    $i16_max = (1 << 15) - 1;
    $this->assertRoundTripI16($i16_min);
    $this->assertRoundTripI16($i16_max);
    $this->assertThrowsOutOfRangeI16($i16_min - 1);
    $this->assertThrowsOutOfRangeI16($i16_max + 1);
  }

  private function assertRoundTripI32(int $value): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $s = CompactTestStruct::withDefaultValues();
      $s->i32_test = $value;
      $serialized =
        TCompactSerializer::serialize($s, null, $disable_hphp_extension);
      $result = TCompactSerializer::deserialize(
        $serialized,
        CompactTestStruct::withDefaultValues(),
        null,
        $disable_hphp_extension,
      );
      expect($value)->toBePHPEqual($result->i32_test);
    }
  }

  private function assertThrowsOutOfRangeI32(int $value): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $s = CompactTestStruct::withDefaultValues();
      $s->i32_test = $value;
      expect(() ==> {
        TCompactSerializer::serialize($s, null, $disable_hphp_extension);
      })->toThrow(TProtocolException::class, null, 'Value is out of range');
    }
  }

  public function testI32Range(): void {
    $i32_min = -(1 << 31);
    $i32_max = (1 << 31) - 1;
    $this->assertRoundTripI32($i32_min);
    $this->assertRoundTripI32($i32_max);
    $this->assertThrowsOutOfRangeI32($i32_min - 1);
    $this->assertThrowsOutOfRangeI32($i32_max + 1);
  }

  public static function provideTestByteRange(): dict<string, (int, int)> {
    return dict[
      'min_byte' => tuple(-128, -128),
      'min_byte_minus_1' => tuple(-129, 127),
      'max_byte' => tuple(127, 127),
      'max_byte_plus_1' => tuple(128, -128),
    ];
  }

  <<DataProvider('provideTestByteRange')>>
  public function testByteRange(int $input, int $expected): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $s = CompactTestStruct::withDefaultValues();
      $s->byte_test = $input;
      $serialized =
        TCompactSerializer::serialize($s, null, $disable_hphp_extension);
      $result = TCompactSerializer::deserialize(
        $serialized,
        CompactTestStruct::withDefaultValues(),
        null,
        $disable_hphp_extension,
      );
      expect($result->byte_test)->toEqual($expected);
    }
  }

  public function testOrder(): void {
    $data = shape('a' => 10, 'b' => 20, 'c' => 30);
    $item1 = OrderedInSerialization::fromShape($data);
    $item2 = Ordered::fromShape($data);
    $item3 = Unordered::fromShape($data);

    $bytes1 = TCompactSerializer::serialize($item1);
    $bytes2 = TCompactSerializer::serialize($item2);
    $bytes3 = TCompactSerializer::serialize($item3);

    expect($bytes1)->toBePHPEqual($bytes2);
    expect(Str\length($bytes1) + 1)->toBePHPEqual(Str\length($bytes3));
  }

  public function testDeserializeCompactMapData(): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      // Test deserialization of a map with one element (key: 1, value: "a")
      $start = "0";
      $type_code = "b";
      $field_id = "20";
      $length = "01";
      $map_key_type = "5";
      $map_value_type = "8";
      $key1 = "02";
      $value1_length = "01";
      $value1_payload = "61";
      $stop = "00";

      $encoded_single_element_map = $start.
        $type_code.
        $field_id.
        $length.
        $map_key_type.
        $map_value_type.
        $key1.
        $value1_length.
        $value1_payload.
        $stop;

      $serialized_single_element_map =
        PHP\pack('H*', $encoded_single_element_map);
      $result = TCompactSerializer::deserialize(
        $serialized_single_element_map,
        CompactTestStruct::withDefaultValues(),
        null,
        $disable_hphp_extension,
      );
      expect(($result->empty_map as nonnull)[1])->toEqual("a");

      // Test deserialization of an empty map (length: 0)
      $zero_length = "00";
      $encoded_empty_map = $start.$type_code.$field_id.$zero_length.$stop;
      $serialized_empty_map = PHP\pack('H*', $encoded_empty_map);
      $result_empty = TCompactSerializer::deserialize(
        $serialized_empty_map,
        CompactTestStruct::withDefaultValues(),
        null,
        $disable_hphp_extension,
      );
      expect($result_empty->empty_map)->toNotBeNull();
    }
  }

  public function testDeserializeBinaryMapData(): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      // Test deserialization of a map with one element (key: 1, value: "a")
      $type_code = "0d";
      $field_id = "0010";
      $length = "00000001";
      $map_key_type = "08";
      $map_value_type = "0b";
      $key1 = "00000001";
      $value1_length = "00000001";
      $value1_payload = "61";
      $stop = "00";

      $encoded_single_element_map = $type_code.
        $field_id.
        $map_key_type.
        $map_value_type.
        $length.
        $key1.
        $value1_length.
        $value1_payload.
        $stop;

      $serialized_single_element_map =
        PHP\pack('H*', $encoded_single_element_map);
      $result = TBinarySerializer::deserialize(
        $serialized_single_element_map,
        CompactTestStruct::withDefaultValues(),
        $disable_hphp_extension,
      );
      expect(($result->empty_map as nonnull)[1])->toEqual("a");

      // Test deserialization of an empty map (length: 0)
      $zero_length = "00000000";
      $encoded_empty_map =
        $type_code.$field_id.$map_key_type.$map_value_type.$zero_length.$stop;
      $serialized_empty_map = PHP\pack('H*', $encoded_empty_map);
      $result_empty = TBinarySerializer::deserialize(
        $serialized_empty_map,
        CompactTestStruct::withDefaultValues(),
        $disable_hphp_extension,
      );
      expect($result_empty->empty_map)->toNotBeNull();
    }
  }
}
