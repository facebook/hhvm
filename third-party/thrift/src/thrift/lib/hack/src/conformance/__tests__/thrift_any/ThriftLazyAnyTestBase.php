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
abstract class ThriftLazyAnyTestBase<T> extends WWWTest {
  const type TProtocolTestDetails = shape(
    'use_json_struct' => bool,
    'serializer' => classname<TProtocolSerializer>,
    'protocol' => apache_thrift_type_standard_StandardProtocol,
  );

  const type TPerProtocolSerializedStrings = dict<string, string>;

  const self::TPerProtocolSerializedStrings
    CPP_HEX_BINARY_SERIALIZED_EMPTY_ANY = dict[
      "json" => "7b226669656c64223a7b2264617461223a22227d7d",
      "binary" => "0c00010b0003000000000000",
      "compact" => "1c38000000",
    ];

  abstract const ThriftStructGenericSpecImpl TYPE_SPEC;

  abstract const self::TPerProtocolSerializedStrings
    CPP_HEX_BINARY_SERIALIZED_STRINGS;

  abstract protected static function getExampleValue(): T;

  abstract protected static function getEmptyValue(): ?T;

  abstract protected static function getValueFromLazyAny(
    ThriftLazyAny $any,
  ): ?T;

  // ==================== Data Provider Helpers ====================
  <<__Memoize>>
  protected static function getProtocolDetails(
  )[]: dict<string, self::TProtocolTestDetails> {
    return dict[
      'json' => shape(
        'use_json_struct' => true,
        'serializer' => JSONThriftSerializer::class,
        'protocol' => apache_thrift_type_standard_StandardProtocol::SimpleJson,
      ),
      'binary' => shape(
        'use_json_struct' => false,
        'serializer' => TBinarySerializer::class,
        'protocol' => apache_thrift_type_standard_StandardProtocol::Binary,
      ),
      'compact' => shape(
        'use_json_struct' => false,
        'serializer' => TCompactSerializer::class,
        'protocol' => apache_thrift_type_standard_StandardProtocol::Compact,
      ),
    ];
  }

  <<__NeedsConcrete>>
  protected static function getExpectedSerialized(
    self::TProtocolTestDetails $info,
  ): string {
    $serializer = $info['serializer'];
    return $serializer::serialize(
      facebook\thrift\test\AnyTestHelper::fromShape(shape(
        'field' => apache_thrift_type_AnyStruct::fromShape(shape(
          'type' => ThriftTypeStructAdapter::fromTypeSpec(static::TYPE_SPEC),
          'protocol' => apache_thrift_type_rep_ProtocolUnion::fromShape(
            shape('standard' => $info['protocol']),
          ),
          'data' => $serializer::serializeData(
            static::getExampleValue(),
            static::TYPE_SPEC,
          ),
        )),
      )),
    );
  }

  protected static function getRoundTripTestStruct(
    self::TProtocolTestDetails $info,
    string $expected_serialized,
    bool $force_any_deserialization = false,
  ): IThriftStruct {
    $serializer = $info['serializer'];
    $roundtrip_struct = $serializer::deserialize(
      $expected_serialized,
      $info['use_json_struct']
        ? facebook\thrift\test\MainStructSimpleJson::withDefaultValues()
        : facebook\thrift\test\MainStruct::withDefaultValues(),
    );
    if ($force_any_deserialization) {
      $roundtrip_struct->field
        ?->getNullableUntyped();
    }
    return $roundtrip_struct;
  }

  <<__NeedsConcrete>>
  protected static function getInitializedTestStruct(
    self::TProtocolTestDetails $info,
  ): IThriftStruct {
    // get the outter class name
    $obj = $info['use_json_struct']
      ? facebook\thrift\test\MainStructSimpleJson::withDefaultValues()
      : facebook\thrift\test\MainStruct::withDefaultValues();
    $lazy_any = ThriftLazyAny::fromObjectWithExplicitTypeStruct(
      static::getExampleValue(),
      ThriftTypeStructAdapter::fromTypeSpec(static::TYPE_SPEC),
    );

    if (
      $info['protocol'] === apache_thrift_type_standard_StandardProtocol::Binary
    ) {
      // Adapter cannot support converting to any using binary serializer.
      // So Force conversion to Any using the test specific protocol
      // This is to ensure that $expected_serialized and $initialized_struct
      // use the same serializer.
      $lazy_any =
        ThriftLazyAny::fromAnyStruct($lazy_any->toAnyUsingBinarySerializer());
    } else {
      $lazy_any->toAny($info['protocol']);
    }
    return $obj::fromShape(
      shape(
        'field' => $lazy_any,
      ),
    );
  }

  // ==================== Data Provider =====================

  <<__NeedsConcrete>>
  public static function getTestCases(
  ): dict<string, (IThriftStruct, T, string, classname<TProtocolSerializer>)> {
    $protocol_test_set = static::getProtocolDetails();
    $test_cases = dict[];
    foreach ($protocol_test_set as $prot => $details) {
      $expected_serialized = static::getExpectedSerialized($details);
      $roundtrip_struct =
        static::getRoundTripTestStruct($details, $expected_serialized);
      // This is equal to $roundtrip_struct above but must be a separate object
      // to avoid interference between different test scenarios.
      $deserialized_roundtrip_struct =
        static::getRoundTripTestStruct($details, $expected_serialized, true);
      invariant(
        $roundtrip_struct !== $deserialized_roundtrip_struct,
        '$roundtrip_struct should be a different object to $deserialized_roundtrip_struct',
      );
      $s = $details['serializer'];
      $test_cases[$prot.'__assigned'] = tuple(
        static::getInitializedTestStruct($details),
        static::getExampleValue(),
        $expected_serialized,
        $s,
      );
      $test_cases[$prot.'__roundtrip'] = tuple(
        $roundtrip_struct,
        static::getExampleValue(),
        $expected_serialized,
        $s,
      );
      $test_cases[$prot.'__roundtrip_deserialized'] = tuple(
        $deserialized_roundtrip_struct,
        static::getExampleValue(),
        $expected_serialized,
        $s,
      );
    }
    return $test_cases;
  }

  <<__NeedsConcrete>>
  public static function getRoundTripTestCases(
  ): dict<string, (IThriftStruct, T)> {
    $protocol_test_set = static::getProtocolDetails();
    $test_cases = dict[];
    foreach ($protocol_test_set as $prot => $details) {
      $expected_serialized = static::getExpectedSerialized($details);
      $roundtrip_struct =
        static::getRoundTripTestStruct($details, $expected_serialized);
      $test_cases[$prot] = tuple($roundtrip_struct, static::getExampleValue());
    }
    return $test_cases;
  }

  <<__NeedsConcrete>>
  public static function getBackwardsCompatibilityTestCases(
  ): dict<string, (IThriftStruct, string, string)> {
    $protocol_test_set = static::getProtocolDetails();
    $test_cases = dict[];
    foreach ($protocol_test_set as $prot => $details) {
      $obj = $details['use_json_struct']
        ? facebook\thrift\test\MainStructSimpleJson::withDefaultValues()
        : facebook\thrift\test\MainStruct::withDefaultValues();
      $struct = static::getInitializedTestStruct($details);
      $serializer = $details['serializer'];

      $deserialized_struct = $serializer::deserialize(
        PHP\hex2bin(static::CPP_HEX_BINARY_SERIALIZED_STRINGS[$prot]),
        $obj,
      );

      $test_cases[$prot] = tuple(
        $deserialized_struct,
        $serializer::serialize($struct),
        static::CPP_HEX_BINARY_SERIALIZED_STRINGS[$prot],
      );
    }
    return $test_cases;
  }

  // It is not normally possible to create such a value using the public
  // LazyAny API in Hack but it might come over the wire from a C++ client.
  public static function getEmptyAnyTestCases(
  ): dict<string, (IThriftStruct, string, string)> {
    $protocol_test_set = static::getProtocolDetails();
    $test_cases = dict[];
    foreach ($protocol_test_set as $prot => $details) {
      $obj = $details['use_json_struct']
        ? facebook\thrift\test\MainStructSimpleJson::withDefaultValues()
        : facebook\thrift\test\MainStruct::withDefaultValues();
      $serializer = $details['serializer'];
      $serialized =
        (string)PHP\hex2bin(static::CPP_HEX_BINARY_SERIALIZED_EMPTY_ANY[$prot]);
      $struct = $serializer::deserialize($serialized, $obj);

      $test_cases[$prot] =
        tuple($struct, $serialized, $serializer::serialize($struct));
    }
    return $test_cases;
  }

  // ==================== Test Cases ====================

  protected static function getTLazyAnyFromThriftStruct(
    IThriftStruct $test_struct,
  ): ThriftLazyAny {
    invariant(
      $test_struct is facebook\thrift\test\MainStruct ||
        $test_struct is facebook\thrift\test\MainStructSimpleJson,
      'Invalid structure type',
    );
    $lazy = $test_struct->field as nonnull;
    return $lazy;
  }

  <<DataProvider('getTestCases')>>
  public function testGet(
    IThriftStruct $test_struct,
    T $expected_value,
    string $_expected_serialized,
    classname<TProtocolSerializer> $_serializer,
  ): void {
    $lazy = static::getTLazyAnyFromThriftStruct($test_struct);
    expect(static::getValueFromLazyAny($lazy))
      ->toBePHPEqual($expected_value);
    expect(static::getValueFromLazyAny($lazy))
      ->toEqual(static::getValueFromLazyAny($lazy));
  }

  <<DataProvider('getTestCases')>>
  public function testGetUntyped(
    IThriftStruct $test_struct,
    T $expected_value,
    string $_expected_serialized,
    classname<TProtocolSerializer> $_serializer,
  ): void {
    $lazy = self::getTLazyAnyFromThriftStruct($test_struct);
    expect($lazy->getNullableUntyped())
      ->toBePHPEqual($expected_value);
    expect($lazy->getNullableUntyped())
      ->toEqual(static::getValueFromLazyAny($lazy));
  }

  <<DataProvider('getTestCases')>>
  public function testSerialize(
    IThriftStruct $test_struct,
    T $_expected_value,
    string $expected_serialized,
    classname<TProtocolSerializer> $serializer,
  ): void {
    invariant(
      $test_struct is facebook\thrift\test\MainStruct ||
        $test_struct is facebook\thrift\test\MainStructSimpleJson,
      'Invalid object',
    );
    expect($serializer::serialize($test_struct))->toEqual($expected_serialized);
  }

  <<DataProvider('getTestCases')>>
  public function testWrongType(
    IThriftStruct $test_struct,
    T $expected_value,
    string $_expected_serialized,
    classname<TProtocolSerializer> $_serializer,
  ): void {
    $lazy = self::getTLazyAnyFromThriftStruct($test_struct);
    expect(() ==> $lazy->get<facebook\thrift\test\DifferentStruct>())
      ->toThrow(Exception::class);
    expect(() ==> static::getValueFromLazyAny($lazy))
      ->notToThrow();
    expect(() ==> $lazy->get<facebook\thrift\test\DifferentStruct>())
      ->toThrow(Exception::class);
    expect(static::getValueFromLazyAny($lazy))
      ->toBePHPEqual($expected_value);
  }

  public function testJSCall(): void {
    $payload = JS::withJSCapture(($context) ==> {
      $context->addJSCall(
        JS\ExampleJSController::emptyFunction(static::getExampleValue()),
      );
      $context->addJSCall(
        JS\ExampleJSController::emptyFunction(
          ThriftLazyAny::fromObjectWithExplicitTypeStruct(
            static::getExampleValue(),
            ThriftTypeStructAdapter::fromTypeSpec(static::TYPE_SPEC),
          ),
        ),
      );
      return $context->getPayload();
    });
    expect($payload->getRequires()->count())
      ->toEqual(2);
    expect($payload->getRequires()[0][3])
      ->toBePHPEqual($payload->getRequires()[1][3]);
  }

  <<DataProvider('getRoundTripTestCases')>>
  public function testRoundTrip(
    IThriftStruct $struct,
    T $expected_value,
  ): void {
    $lazy = static::getTLazyAnyFromThriftStruct($struct);
    $data = static::getValueFromLazyAny($lazy);
    expect($data)->toBePHPEqual($expected_value);
  }

  <<DataProvider('getBackwardsCompatibilityTestCases')>>
  public function testBackwardsCompatibility(
    IThriftStruct $struct,
    string $serialized_struct,
    string $cpp_hex_bin_string,
  ): void {
    expect($serialized_struct)->toEqual(PHP\hex2bin($cpp_hex_bin_string));
    $lazy = static::getTLazyAnyFromThriftStruct($struct);
    $data = static::getValueFromLazyAny($lazy);
    expect($data)->toBePHPEqual(static::getExampleValue());
  }

  <<DataProvider('getEmptyAnyTestCases')>>
  public function testEmptyAny(
    IThriftStruct $struct,
    string $serialized_struct,
    string $expected_serialized,
  ): void {
    $lazy = static::getTLazyAnyFromThriftStruct($struct);
    $data = static::getValueFromLazyAny($lazy);
    expect($data)->toBePHPEqual(static::getEmptyValue());
    expect($serialized_struct)->toEqual($expected_serialized);
  }
}
