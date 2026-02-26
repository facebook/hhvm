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

<<Oncalls('search_topaggr'), Feature('FBApp_Search_IndexServe')>>
abstract class LazyAnyTestBase extends WWWTest {

  abstract const class<IThriftStruct> CLASSNAME;
  abstract const classname<TProtocolSerializer> DATA_SERIALIZER;
  abstract const ?apache_thrift_StandardProtocol THRIFT_PROTOCOL;
  abstract const string CPP_HEX_BINARY_SERIALIZED;

  private static function getClassnameObject(): IThriftStruct {
    $t = static::CLASSNAME;
    return new $t();
  }

  public function testRoundtrip(): void {
    $any = LazyAny::fromStruct(
      unicorn_test_ExampleStruct::fromShape(shape('num' => 5)),
    );
    $value1 = $any->get(unicorn_test_ExampleStruct::class);
    $value1->vec = vec["foo", "bar"];

    $obj = self::getClassnameObject();
    invariant(
      $obj is unicorn_test_MainStruct ||
        $obj is unicorn_test_MainStructSimpleJson,
      'Invalid object. It should be unicorn_test_MainStruct or unicorn_test_MainStructSimpleJson',
    );
    $roundtrip = TCompactSerializer::deserialize(
      TCompactSerializer::serialize($obj::fromShape(shape('field' => $any))),
      $obj,
    );

    $value2 =
      ($roundtrip->field as nonnull)->get(unicorn_test_ExampleStruct::class);
    expect($value2->num)->toEqual(5);
    expect($value2->vec)->toEqual(vec["foo", "bar"]);
    expect($value2)->toBePHPEqual($value1);
  }

  public function testBackwardsCompatibility1(): void {
    $obj = self::getClassnameObject();
    invariant(
      $obj is unicorn_test_MainStruct ||
        $obj is unicorn_test_MainStructSimpleJson,
      'Invalid object. It should be unicorn_test_MainStruct or unicorn_test_MainStructSimpleJson',
    );
    $test = $obj::fromShape(shape(
      'field' => LazyAny::fromStruct(
        unicorn_test_ExampleStruct::fromShape(
          shape('num' => 5, 'vec' => vec["foo", "bar"]),
        ),
      ),
    ));
    expect(TCompactSerializer::serialize($test))->toEqual(
      PHP\hex2bin(static::CPP_HEX_BINARY_SERIALIZED),
    );
  }

  public function testBackwardsCompatibility2(): void {
    $obj = self::getClassnameObject();
    invariant(
      $obj is unicorn_test_MainStruct ||
        $obj is unicorn_test_MainStructSimpleJson,
      'Invalid object. It should be unicorn_test_MainStruct or unicorn_test_MainStructSimpleJson',
    );
    $test = TCompactSerializer::deserialize(
      PHP\hex2bin(static::CPP_HEX_BINARY_SERIALIZED),
      $obj,
    );

    $value = ($test->field as nonnull)->get(unicorn_test_ExampleStruct::class);
    expect($value->num)->toEqual(5);
    expect($value->vec)->toEqual(vec["foo", "bar"]);
  }

  public function testEmptyAny(): void {
    // It is not normally possible to create such a value using the public
    // LazyAny API in Hack but it might come over the wire from a C++ client.
    // Here "1c58000000" is the output of serialize(MainStruct{}) in C++.
    $obj = self::getClassnameObject();
    invariant(
      $obj is unicorn_test_MainStruct ||
        $obj is unicorn_test_MainStructSimpleJson,
      'Invalid object. It should be unicorn_test_MainStruct or unicorn_test_MainStructSimpleJson',
    );
    $serialized = PHP\hex2bin("1c58000000");
    $test = TCompactSerializer::deserialize($serialized, $obj);
    expect(TCompactSerializer::serialize($test))->toEqual($serialized);
    expect($test->field?->get(unicorn_test_ExampleStruct::class))->toBePHPEqual(
      unicorn_test_ExampleStruct::withDefaultValues(),
    );
  }

  public static function getTestCases(
  ): dict<string, (mixed, unicorn_test_ExampleStruct, string)> {
    // Inner class represented by LazyAny
    $expected_value = unicorn_test_ExampleStruct::fromShape(
      shape('num' => 5, 'vec' => vec["foo", "bar"]),
    );
    // get the outter class name
    $obj = self::getClassnameObject();
    invariant(
      $obj is unicorn_test_MainStruct ||
        $obj is unicorn_test_MainStructSimpleJson,
      'Invalid object. It should be unicorn_test_MainStruct or unicorn_test_MainStructSimpleJson',
    );
    $initialized_struct =
      $obj::fromShape(shape('field' => LazyAny::fromStruct($expected_value)));
    $s = static::DATA_SERIALIZER;
    $expected_serialized = TCompactSerializer::serialize(
      unicorn_test_AnyTestHelper::fromShape(shape(
        'field' => apache_thrift_Any::fromShape(shape(
          'typeHashPrefixSha2_256' => Str\slice(
            BypassVisibility::invokeStaticMethod(
              ThriftTypeRegistry::class,
              'getHashForHackType',
              unicorn_test_ExampleStruct::class,
            ),
            0,
            ThriftUniversalName::DEFAULT_HASH_LENGTH,
          ),
          'protocol' => static::THRIFT_PROTOCOL,
          'data' => $s::serialize($expected_value),
        )),
      )),
    );
    $roundtrip_struct =
      TCompactSerializer::deserialize($expected_serialized, $obj);
    // This is equal to $roundtrip_struct above but must be a separate object
    // to avoid interference between different test scenarios.
    $deserialized_roundtrip_struct =
      TCompactSerializer::deserialize($expected_serialized, $obj);
    invariant(
      $roundtrip_struct !== $deserialized_roundtrip_struct,
      '$roundtrip_struct should be a different object to $deserialized_roundtrip_struct',
    );
    $deserialized_roundtrip_struct->field
      ?->get(unicorn_test_ExampleStruct::class); // Force deserialization.

    return dict[
      'assigned' =>
        tuple($initialized_struct, $expected_value, $expected_serialized),
      'roundtrip' =>
        tuple($roundtrip_struct, $expected_value, $expected_serialized),
      'roundtrip_deserialized' => tuple(
        $deserialized_roundtrip_struct,
        $expected_value,
        $expected_serialized,
      ),
    ];
  }

  private static function getLazyAnyFromMixed(mixed $test_struct): LazyAny {
    invariant(
      $test_struct is unicorn_test_MainStruct ||
        $test_struct is unicorn_test_MainStructSimpleJson,
      'Invalid structure type',
    );
    $lazy = $test_struct->field as nonnull;
    return $lazy;
  }

  <<DataProvider('getTestCases')>>
  public function testGet(
    mixed $test_struct,
    unicorn_test_ExampleStruct $expected_value,
    string $_expected_serialized,
  ): void {
    $lazy = self::getLazyAnyFromMixed($test_struct);
    expect($lazy->get(unicorn_test_ExampleStruct::class))
      ->toBePHPEqual($expected_value);
    expect($lazy->get(unicorn_test_ExampleStruct::class))
      ->toEqual($lazy->get(unicorn_test_ExampleStruct::class));
  }

  <<DataProvider('getTestCases')>>
  public function testGetReified(
    mixed $test_struct,
    unicorn_test_ExampleStruct $expected_value,
    string $_expected_serialized,
  ): void {
    $lazy = self::getLazyAnyFromMixed($test_struct);
    expect($lazy->getReified<unicorn_test_ExampleStruct>())
      ->toBePHPEqual($expected_value);
    expect($lazy->getReified<unicorn_test_ExampleStruct>())
      ->toEqual($lazy->getReified<unicorn_test_ExampleStruct>());
  }

  <<DataProvider('getTestCases')>>
  public function testGetUntyped(
    mixed $test_struct,
    unicorn_test_ExampleStruct $expected_value,
    string $_expected_serialized,
  ): void {
    $lazy = self::getLazyAnyFromMixed($test_struct);
    expect($lazy->getUntyped())
      ->toBePHPEqual($expected_value);
    expect($lazy->getUntyped())
      ->toEqual($lazy->get(unicorn_test_ExampleStruct::class));
  }

  <<DataProvider('getTestCases')>>
  public function testSerialize(
    mixed $test_struct,
    unicorn_test_ExampleStruct $_expected_value,
    string $expected_serialized,
  ): void {
    invariant(
      $test_struct is unicorn_test_MainStruct ||
        $test_struct is unicorn_test_MainStructSimpleJson,
      'Invalid object',
    );
    expect(TCompactSerializer::serialize($test_struct))->toEqual(
      $expected_serialized,
    );
  }

  <<DataProvider('getTestCases')>>
  public function testWrongType(
    mixed $test_struct,
    unicorn_test_ExampleStruct $expected_value,
    string $_expected_serialized,
  ): void {
    $lazy = self::getLazyAnyFromMixed($test_struct);
    expect(() ==> $lazy->get(unicorn_test_DifferentStruct::class))
      ->toThrow(Exception::class);
    expect(() ==> $lazy->get(unicorn_test_ExampleStruct::class))
      ->notToThrow();
    expect(() ==> $lazy->get(unicorn_test_DifferentStruct::class))
      ->toThrow(Exception::class);
    expect($lazy->get(unicorn_test_ExampleStruct::class))
      ->toBePHPEqual($expected_value);
  }

  public function testJSCall(): void {
    $struct = unicorn_test_ExampleStruct::fromShape(
      shape('num' => 5, 'vec' => vec["foo", "bar"]),
    );
    $payload = JS::withJSCapture(($context) ==> {
      $context->addJSCall(JS\ExampleJSController::emptyFunction($struct));
      $context->addJSCall(
        JS\ExampleJSController::emptyFunction(LazyAny::fromStruct($struct)),
      );
      return $context->getPayload();
    });
    expect($payload->getRequires()->count())
      ->toEqual(2);
    expect($payload->getRequires()[0][3])
      ->toBePHPEqual($payload->getRequires()[1][3]);
  }

}
