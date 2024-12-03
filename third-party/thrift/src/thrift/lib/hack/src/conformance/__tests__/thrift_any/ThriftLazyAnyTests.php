<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
final class ThriftLazyAnyTestForStruct
  extends ThriftLazyAnyTestBase<facebook\thrift\test\ExampleStruct> {

  const ThriftStructGenericSpecImpl TYPE_SPEC = shape(
    'type' => TType::STRUCT,
    'class' => facebook\thrift\test\ExampleStruct::class,
  );

  const self::TPerProtocolSerializedStrings CPP_HEX_BINARY_SERIALIZED_STRINGS =
    dict[
      "json" =>
        "7b226669656c64223a7b2274797065223a7b226e616d65223a7b2273747275637454797065223a7b22757269223a2266616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c65537472756374227d7d2c22706172616d73223a5b5d7d2c2270726f746f636f6c223a7b227374616e64617264223a347d2c2264617461223a227b5c226e756d5c223a352c5c227665635c223a5b5c22666f6f5c222c5c226261725c225d7d227d7d",
      "binary" =>
        "0c00010c00010c00010c000b0b00010000002666616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c6553747275637400000f00020c00000000000c000208000100000001000b00030000001e080001000000050f00030b0000000200000003666f6f00000003626172000000",
      "compact" =>
        "1c1c1cbc182666616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c655374727563740000190c001c150400180d150a292803666f6f03626172000000",
    ];

  <<__Override>>
  protected static function getExampleValue(
  ): facebook\thrift\test\ExampleStruct {
    return facebook\thrift\test\ExampleStruct::fromShape(
      shape('num' => 5, 'vec' => vec["foo", "bar"]),
    );
  }

  <<__Override>>
  protected static function getEmptyValue(
  ): facebook\thrift\test\ExampleStruct {
    return facebook\thrift\test\ExampleStruct::withDefaultValues();
  }

  <<__Override>>
  protected static function getValueFromLazyAny(
    ThriftLazyAny $any,
  ): facebook\thrift\test\ExampleStruct {
    return $any->get<facebook\thrift\test\ExampleStruct>();
  }
}

<<Oncalls('thrift')>>
final class ThriftLazyAnyTestForString extends ThriftLazyAnyTestBase<string> {

  const ThriftStructGenericSpecImpl TYPE_SPEC = shape('type' => TType::STRING);

  const self::TPerProtocolSerializedStrings CPP_HEX_BINARY_SERIALIZED_STRINGS =
    dict[
      "json" =>
        "7b226669656c64223a7b2274797065223a7b226e616d65223a7b22737472696e6754797065223a307d2c22706172616d73223a5b5d7d2c2270726f746f636f6c223a7b227374616e64617264223a347d2c2264617461223a225c2268656c6c6f20776f726c645c22227d7d",
      "binary" =>
        "0c00010c00010c000108000800000000000f00020c00000000000c000208000100000001000b00030000000f0000000b68656c6c6f20776f726c640000",
      "compact" => "1c1c1c850000190c001c150400180c0b68656c6c6f20776f726c640000",
    ];

  <<__Override>>
  protected static function getExampleValue(): string {
    return "hello world";
  }

  <<__Override>>
  protected static function getEmptyValue(): string {
    return "";
  }

  <<__Override>>
  protected static function getValueFromLazyAny(ThriftLazyAny $any): ?string {
    return $any->get<string>();
  }
}

<<Oncalls('thrift')>>
final class ThriftLazyAnyTestForEnum
  extends ThriftLazyAnyTestBase<facebook\thrift\test\ExampleEnum> {

  const ThriftStructGenericSpecImpl TYPE_SPEC = shape(
    'type' => TType::I32,
    'enum' => facebook\thrift\test\ExampleEnum::class,
  );

  const self::TPerProtocolSerializedStrings CPP_HEX_BINARY_SERIALIZED_STRINGS =
    dict[
      "json" =>
        "7b226669656c64223a7b2274797065223a7b226e616d65223a7b22656e756d54797065223a7b22757269223a2266616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c65456e756d227d7d2c22706172616d73223a5b5d7d2c2270726f746f636f6c223a7b227374616e64617264223a347d2c2264617461223a2231227d7d",
      "binary" =>
        "0c00010c00010c00010c000a0b00010000002466616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c65456e756d00000f00020c00000000000c000208000100000001000b000300000004000000010000",
      "compact" =>
        "1c1c1cac182466616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c65456e756d0000190c001c1504001801020000",
    ];

  <<__Override>>
  protected static function getExampleValue(
  ): facebook\thrift\test\ExampleEnum {
    return facebook\thrift\test\ExampleEnum::ENUM_VALUE_1;
  }

  <<__Override>>
  protected static function getEmptyValue(): ?facebook\thrift\test\ExampleEnum {
    return facebook\thrift\test\ExampleEnum::ENUM_VALUE_0;
  }

  <<__Override>>
  protected static function getValueFromLazyAny(
    ThriftLazyAny $any,
  ): ?facebook\thrift\test\ExampleEnum {
    return $any->get<facebook\thrift\test\ExampleEnum>();
  }
}

<<Oncalls('thrift')>>
final class ThriftLazyAnyTestForContainer
  extends ThriftLazyAnyTestBase<dict<int, string>> {

  const ThriftStructGenericSpecImpl TYPE_SPEC = shape(
    'type' => TType::MAP,
    'key' => shape('type' => TType::I32),
    'ktype' => TType::I32,
    'val' => shape('type' => TType::STRING),
    'vtype' => TType::STRING,
    'format' => 'harray',
  );

  const self::TPerProtocolSerializedStrings CPP_HEX_BINARY_SERIALIZED_STRINGS =
    dict[
      "json" =>
        "7b226669656c64223a7b2274797065223a7b226e616d65223a7b226d617054797065223a307d2c22706172616d73223a5b7b226e616d65223a7b2269333254797065223a307d2c22706172616d73223a5b5d7d2c7b226e616d65223a7b22737472696e6754797065223a307d2c22706172616d73223a5b5d7d5d7d2c2270726f746f636f6c223a7b227374616e64617264223a347d2c2264617461223a227b5c22315c223a5c2268656c6c6f20776f726c645c227d227d7d",
      "binary" =>
        "0c00010c00010c000108001000000000000f00020c000000020c000108000400000000000f00020c00000000000c000108000800000000000f00020c0000000000000c000208000100000001000b000300000019080b00000001000000010000000b68656c6c6f20776f726c640000",
      "compact" =>
        "1c1c1c05200000192c1c450000190c001c850000190c00001c150400180f0158020b68656c6c6f20776f726c640000",
    ];

  <<__Override>>
  protected static function getExampleValue(): dict<int, string> {
    return dict[1 => "hello world"];
  }

  <<__Override>>
  protected static function getEmptyValue(): dict<int, string> {
    return dict[];
  }

  <<__Override>>
  protected static function getValueFromLazyAny(
    ThriftLazyAny $any,
  ): dict<int, string> {
    return $any->get<dict<int, string>>();
  }
}

<<Oncalls('thrift')>>
final class ThriftLazyAnyTestForBool extends ThriftLazyAnyTestBase<bool> {

  const ThriftStructGenericSpecImpl TYPE_SPEC = shape('type' => TType::BOOL);

  const self::TPerProtocolSerializedStrings CPP_HEX_BINARY_SERIALIZED_STRINGS =
    dict[
      "json" =>
        "7b226669656c64223a7b2274797065223a7b226e616d65223a7b22626f6f6c54797065223a307d2c22706172616d73223a5b5d7d2c2270726f746f636f6c223a7b227374616e64617264223a347d2c2264617461223a2274727565227d7d",
      "binary" =>
        "0c00010c00010c000108000100000000000f00020c00000000000c000208000100000001000b000300000001010000",
      "compact" => "1c1c1c150000190c001c1504001801010000",
    ];

  <<__Override>>
  protected static function getExampleValue(): bool {
    return true;
  }

  <<__Override>>
  protected static function getEmptyValue(): ?bool {
    return false;
  }

  <<__Override>>
  protected static function getValueFromLazyAny(ThriftLazyAny $any): ?bool {
    return $any->get<bool>();
  }
}

<<Oncalls('thrift')>>
final class ThriftLazyAnyTestForNestedStructInContainers
  extends ThriftLazyAnyTestBase<
    dict<int, dict<
      facebook\thrift\test\ExampleEnum,
      facebook\thrift\test\ExampleStruct,
    >>,
  > {
  const type THackType = dict<
    int,
    dict<facebook\thrift\test\ExampleEnum, facebook\thrift\test\ExampleStruct>,
  >;
  const ThriftStructGenericSpecImpl TYPE_SPEC = shape(
    'type' => TType::MAP,
    'key' => shape('type' => TType::I32),
    'ktype' => TType::I32,
    'vtype' => TType::MAP,
    'val' => shape(
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::STRUCT,
      'key' => shape(
        'type' => TType::I32,
        'enum' => facebook\thrift\test\ExampleEnum::class,
      ),
      'val' => shape(
        'type' => TType::STRUCT,
        'class' => facebook\thrift\test\ExampleStruct::class,
      ),
      'format' => 'harray',
    ),
    'format' => 'harray',
  );

  const self::TPerProtocolSerializedStrings CPP_HEX_BINARY_SERIALIZED_STRINGS =
    dict[
      "json" =>
        "7b226669656c64223a7b2274797065223a7b226e616d65223a7b226d617054797065223a307d2c22706172616d73223a5b7b226e616d65223a7b2269333254797065223a307d2c22706172616d73223a5b5d7d2c7b226e616d65223a7b226d617054797065223a307d2c22706172616d73223a5b7b226e616d65223a7b22656e756d54797065223a7b22757269223a2266616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c65456e756d227d7d2c22706172616d73223a5b5d7d2c7b226e616d65223a7b2273747275637454797065223a7b22757269223a2266616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c65537472756374227d7d2c22706172616d73223a5b5d7d5d7d5d7d2c2270726f746f636f6c223a7b227374616e64617264223a347d2c2264617461223a227b5c22315c223a7b5c22305c223a7b5c226e756d5c223a302c5c227665635c223a5b5d7d2c5c22315c223a7b5c226e756d5c223a302c5c227665635c223a5b5c22666f6f5c222c5c226261725c225d7d7d7d227d7d",
      "binary" =>
        "0c00010c00010c000108001000000000000f00020c000000020c000108000400000000000f00020c00000000000c000108001000000000000f00020c000000020c00010c000a0b00010000002466616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c65456e756d00000f00020c00000000000c00010c000b0b00010000002666616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c6553747275637400000f00020c000000000000000c000208000100000001000b000300000046080d0000000100000001080c0000000200000000080001000000000f00030b000000000000000001080001000000000f00030b0000000200000003666f6f00000003626172000000",
      "compact" =>
        "1c1c1c05200000192c1c450000190c001c05200000192c1cac182466616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c65456e756d0000190c001cbc182666616365626f6f6b2e636f6d2f7468726966742f746573742f4578616d706c655374727563740000190c0000001c1504001819015b02025c001500290800021500292803666f6f03626172000000",
    ];

  <<__Override>>
  protected static function getExampleValue(): self::THackType {
    return dict[
      1 => dict[
        facebook\thrift\test\ExampleEnum::ENUM_VALUE_0 =>
          facebook\thrift\test\ExampleStruct::fromShape(shape(
            'num' => 0,
          )),
        facebook\thrift\test\ExampleEnum::ENUM_VALUE_1 =>
          facebook\thrift\test\ExampleStruct::fromShape(shape(
            'vec' => vec['foo', 'bar'],
          )),
      ],
    ];
  }

  <<__Override>>
  protected static function getEmptyValue(): self::THackType {
    return dict[];
  }

  <<__Override>>
  protected static function getValueFromLazyAny(
    ThriftLazyAny $any,
  ): self::THackType {
    return $any->get<self::THackType>();
  }
}
