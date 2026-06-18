<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
final class ThriftKeySerializerTest extends WWWTest {

  public function testEqualThriftValuesSerializeIdentically(): void {
    $spec = $this->simpleStructSpec();
    $a = SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));
    $b = SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));

    expect(ThriftKeySerializer::serialize($a, TType::STRUCT, $spec))
      ->toEqual(ThriftKeySerializer::serialize($b, TType::STRUCT, $spec));
  }

  public function testDistinctThriftValuesSerializeDifferently(): void {
    $spec = $this->simpleStructSpec();
    $a = SimpleStruct::fromShape(shape('a_i32' => 1));
    $b = SimpleStruct::fromShape(shape('a_i32' => 2));

    $serialized_a = ThriftKeySerializer::serialize($a, TType::STRUCT, $spec);
    $serialized_b = ThriftKeySerializer::serialize($b, TType::STRUCT, $spec);

    expect($serialized_a)->toNotEqual($serialized_b);
  }

  public function testListSerializationPreservesOrder(): void {
    $spec = $this->i32ListSpec();

    $serialized_a =
      ThriftKeySerializer::serialize(Vector {1, 2}, TType::LST, $spec);
    $serialized_b =
      ThriftKeySerializer::serialize(Vector {2, 1}, TType::LST, $spec);

    expect($serialized_a)->toNotEqual($serialized_b);
  }

  public function testSetSerializationIsOrderIndependent(): void {
    $spec = $this->i32SetSpec();
    $a = Set {1, 2, 3};
    $b = Set {3, 1, 2};

    expect(ThriftKeySerializer::serialize($a, TType::SET, $spec))
      ->toEqual(ThriftKeySerializer::serialize($b, TType::SET, $spec));
  }

  public function testSetElementsAreSerializedOnce(): void {
    ThriftKeySerializerCountingStringToIntAdapter::reset();
    $spec = shape(
      'type' => TType::SET,
      'etype' => TType::I32,
      'elem' => shape(
        'adapter' => ThriftKeySerializerCountingStringToIntAdapter::class,
        'type' => TType::I32,
      ),
      'format' => 'collection',
    );

    ThriftKeySerializer::serialize(Vector {'2', '1'}, TType::SET, $spec);

    expect(
      ThriftKeySerializerCountingStringToIntAdapter::getToThriftCallCount(),
    )
      ->toEqual(2);
  }

  public function testMapSerializationIsOrderIndependent(): void {
    $spec = $this->i32StringMapSpec();
    $a = Map {1 => 'a', 2 => 'b', 3 => 'c'};
    $b = Map {3 => 'c', 1 => 'a', 2 => 'b'};

    expect(ThriftKeySerializer::serialize($a, TType::MAP, $spec))
      ->toEqual(ThriftKeySerializer::serialize($b, TType::MAP, $spec));
  }

  public function testMapKeysAreSerializedOnce(): void {
    ThriftKeySerializerCountingStringToIntAdapter::reset();
    $spec = shape(
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::BOOL,
      'key' => shape(
        'adapter' => ThriftKeySerializerCountingStringToIntAdapter::class,
        'type' => TType::I32,
      ),
      'val' => shape('type' => TType::BOOL),
      'format' => 'collection',
    );

    ThriftKeySerializer::serialize(
      Map {'2' => false, '1' => true},
      TType::MAP,
      $spec,
    );

    expect(
      ThriftKeySerializerCountingStringToIntAdapter::getToThriftCallCount(),
    )
      ->toEqual(2);
  }

  public function testNestedUnorderedContainersInsideStructAreDeterministic(
  ): void {
    $a = ContainerStruct::withDefaultValues();
    $a->i32_set[] = 1;
    $a->i32_set[] = 2;
    $a->i32_i16_map[1] = 10;
    $a->i32_i16_map[2] = 20;

    $b = ContainerStruct::withDefaultValues();
    $b->i32_set[] = 2;
    $b->i32_set[] = 1;
    $b->i32_i16_map[2] = 20;
    $b->i32_i16_map[1] = 10;

    $spec = shape('type' => TType::STRUCT, 'class' => ContainerStruct::class);
    expect(ThriftKeySerializer::serialize($a, TType::STRUCT, $spec))
      ->toEqual(ThriftKeySerializer::serialize($b, TType::STRUCT, $spec));
  }

  public function testSerializedBytesDeserializeBackToThriftValues(): void {
    $spec = $this->simpleStructSpec();
    $value =
      SimpleStruct::fromShape(shape('a_i32' => 7, 'a_string' => 'seven'));
    $serialized = ThriftKeySerializer::serialize($value, TType::STRUCT, $spec);

    $deserialized = ThriftKeySerializer::deserialize(
      $serialized,
      TType::STRUCT,
      $spec,
    ) as SimpleStruct;

    expect($deserialized)->toBePHPEqual($value);
  }

  public function testSerializedBoolBytesDeserializeBackToBoolValues(): void {
    $spec = $this->boolSpec();
    $serialized_false =
      ThriftKeySerializer::serialize(false, TType::BOOL, $spec);
    $serialized_true = ThriftKeySerializer::serialize(true, TType::BOOL, $spec);

    expect(PHP\bin2hex($serialized_false))->toEqual('02');
    expect(PHP\bin2hex($serialized_true))->toEqual('01');
    expect(
      ThriftKeySerializer::deserialize($serialized_false, TType::BOOL, $spec),
    )
      ->toBeFalse();
    expect(
      ThriftKeySerializer::deserialize($serialized_true, TType::BOOL, $spec),
    )
      ->toBeTrue();
  }

  public function testAdaptedBoolKeysUseThriftFormIdentity(): void {
    $spec = shape(
      'adapter' => ThriftKeySerializerBoolToStringAdapter::class,
      'type' => TType::BOOL,
    );

    $serialized_true =
      ThriftKeySerializer::serialize('true', TType::BOOL, $spec);
    $serialized_yes = ThriftKeySerializer::serialize('yes', TType::BOOL, $spec);
    $serialized_false =
      ThriftKeySerializer::serialize('false', TType::BOOL, $spec);

    expect($serialized_true)->toEqual($serialized_yes);
    expect($serialized_true)->toNotEqual($serialized_false);
    expect(PHP\bin2hex($serialized_true))->toEqual('01');
    expect(PHP\bin2hex($serialized_false))->toEqual('02');
    expect(
      ThriftKeySerializer::deserialize($serialized_true, TType::BOOL, $spec),
    )
      ->toEqual('true');
    expect(
      ThriftKeySerializer::deserialize($serialized_false, TType::BOOL, $spec),
    )
      ->toEqual('false');
  }

  public function testBoolStructFieldsUseFieldEncoding(): void {
    $spec = shape('type' => TType::STRUCT, 'class' => SimpleUnion::class);
    $serialized_false = ThriftKeySerializer::serialize(
      SimpleUnion::fromShape(shape('a_bool' => false)),
      TType::STRUCT,
      $spec,
    );
    $serialized_true = ThriftKeySerializer::serialize(
      SimpleUnion::fromShape(shape('a_bool' => true)),
      TType::STRUCT,
      $spec,
    );

    expect($serialized_false)->toNotEqual($serialized_true);
    expect(PHP\bin2hex($serialized_false))
      ->toNotEqual(PHP\bin2hex(
        ThriftKeySerializer::serialize(false, TType::BOOL, $this->boolSpec()),
      ));
    expect(PHP\bin2hex($serialized_true))
      ->toNotEqual(PHP\bin2hex(
        ThriftKeySerializer::serialize(true, TType::BOOL, $this->boolSpec()),
      ));
  }

  public function testBoolListSerializationPreservesOrder(): void {
    $spec = $this->boolListSpec();

    $serialized_a =
      ThriftKeySerializer::serialize(Vector {false, true}, TType::LST, $spec);
    $serialized_b =
      ThriftKeySerializer::serialize(Vector {true, false}, TType::LST, $spec);

    expect($serialized_a)->toNotEqual($serialized_b);
  }

  public function testBoolSetSerializationIsOrderIndependent(): void {
    $spec = $this->boolObjectKeySetSpec();
    $a = Vector {false, true};
    $b = Vector {true, false};

    expect(ThriftKeySerializer::serialize($a, TType::SET, $spec))
      ->toEqual(ThriftKeySerializer::serialize($b, TType::SET, $spec));
  }

  public function testBoolMapSerializationIsOrderIndependent(): void {
    $spec = $this->boolBoolObjectKeyMapSpec();
    $a = new ThriftKeySerializerBoolKeyIterable(
      vec[tuple(false, true), tuple(true, false)],
    );
    $b = new ThriftKeySerializerBoolKeyIterable(
      vec[tuple(true, false), tuple(false, true)],
    );

    expect(ThriftKeySerializer::serialize($a, TType::MAP, $spec))
      ->toEqual(ThriftKeySerializer::serialize($b, TType::MAP, $spec));
  }

  public function testAdapterBackedKeysUseThriftFormIdentity(): void {
    $spec = shape(
      'adapter' => AdapterTestIntToString::class,
      'type' => TType::I32,
    );

    $serialized_42 = ThriftKeySerializer::serialize('42', TType::I32, $spec);
    $serialized_042 = ThriftKeySerializer::serialize('042', TType::I32, $spec);
    $serialized_43 = ThriftKeySerializer::serialize('43', TType::I32, $spec);

    expect($serialized_42)->toEqual($serialized_042);
    expect($serialized_42)->toNotEqual($serialized_43);
    expect(ThriftKeySerializer::deserialize($serialized_42, TType::I32, $spec))
      ->toEqual('42');
  }

  public function testTerseNestedWrappedStructUsesWrappedDefault(): void {
    $spec = shape(
      'type' => TType::STRUCT,
      'class' => ThriftKeySerializerOuterTerseStruct::class,
    );

    $serialized = ThriftKeySerializer::serialize(
      ThriftKeySerializerOuterTerseStruct::withDefaultValues(),
      TType::STRUCT,
      $spec,
    );

    expect(PHP\bin2hex($serialized))->toEqual('00');
  }

  private function simpleStructSpec(): ThriftStructTypes::TGenericSpec {
    return shape('type' => TType::STRUCT, 'class' => SimpleStruct::class);
  }

  private function i32ListSpec(): ThriftStructTypes::TGenericSpec {
    return shape(
      'type' => TType::LST,
      'etype' => TType::I32,
      'elem' => shape('type' => TType::I32),
      'format' => 'collection',
    );
  }

  private function i32SetSpec(): ThriftStructTypes::TGenericSpec {
    return shape(
      'type' => TType::SET,
      'etype' => TType::I32,
      'elem' => shape('type' => TType::I32),
      'format' => 'collection',
    );
  }

  private function i32StringMapSpec(): ThriftStructTypes::TGenericSpec {
    return shape(
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::STRING,
      'key' => shape('type' => TType::I32),
      'val' => shape('type' => TType::STRING),
      'format' => 'collection',
    );
  }

  private function boolSpec(): ThriftStructTypes::TGenericSpec {
    return shape('type' => TType::BOOL);
  }

  private function boolListSpec(): ThriftStructTypes::TGenericSpec {
    return shape(
      'type' => TType::LST,
      'etype' => TType::BOOL,
      'elem' => shape('type' => TType::BOOL),
      'format' => 'collection',
    );
  }

  private function boolObjectKeySetSpec(): ThriftStructTypes::TGenericSpec {
    return shape(
      'type' => TType::SET,
      'etype' => TType::BOOL,
      'elem' => shape('type' => TType::BOOL),
      'format' => 'object_key',
    );
  }

  private function boolBoolObjectKeyMapSpec(): ThriftStructTypes::TGenericSpec {
    return shape(
      'type' => TType::MAP,
      'ktype' => TType::BOOL,
      'vtype' => TType::BOOL,
      'key' => shape('type' => TType::BOOL),
      'val' => shape('type' => TType::BOOL),
      'format' => 'object_key',
    );
  }
}

final class ThriftKeySerializerBoolToStringAdapter implements IThriftAdapter {

  const type TThriftType = bool;
  const type THackType = string;

  public static function fromThrift(bool $thrift_value)[]: string {
    return $thrift_value ? 'true' : 'false';
  }

  public static function toThrift(string $hack_value)[]: bool {
    return $hack_value === 'true' || $hack_value === 'yes';
  }
}

final class ThriftKeySerializerCountingStringToIntAdapter
  implements IThriftAdapter {

  const type TThriftType = int;
  const type THackType = string;

  private static int $toThriftCallCount = 0;

  public static function reset()[write_props]: void {
    HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
      ()[defaults] ==> {
        self::$toThriftCallCount = 0;
      },
      'Test adapter resets static call count.',
    );
  }

  public static function getToThriftCallCount()[write_props]: int {
    return HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
      ()[defaults] ==> self::$toThriftCallCount,
      'Test adapter reads static call count.',
    );
  }

  public static function fromThrift(int $thrift_value)[write_props]: string {
    return (string)$thrift_value;
  }

  public static function toThrift(string $hack_value)[write_props]: int {
    HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
      ()[defaults] ==> {
        self::$toThriftCallCount++;
      },
      'Test adapter counts static toThrift calls.',
    );
    return (int)$hack_value;
  }
}

final class ThriftKeySerializerBoolKeyIterable
  implements KeyedIterable<bool, bool> {
  use StrictKeyedIterable<bool, bool>;

  public function __construct(private vec<(bool, bool)> $entries)[] {}

  public function getIterator()[]: KeyedIterator<bool, bool> {
    return new ThriftKeySerializerBoolKeyIterator($this->entries);
  }
}

final class ThriftKeySerializerBoolKeyIterator
  implements KeyedIterator<bool, bool> {

  private int $idx = 0;

  public function __construct(private vec<(bool, bool)> $entries)[] {}

  public function current(): bool {
    return $this->entries[$this->idx][1];
  }

  public function key(): bool {
    return $this->entries[$this->idx][0];
  }

  public function next(): void {
    $this->idx++;
  }

  public function rewind(): void {
    $this->idx = 0;
  }

  public function valid(): bool {
    return C\contains_key($this->entries, $this->idx);
  }
}

final class ThriftKeySerializerTestI64Wrapper extends IThriftTypeWrapper<int> {

  <<__Override>>
  public static async function genFromThrift<<<__Explicit>> TThriftType__>(
    TThriftType__ $_value,
  )[zoned_shallow]: Awaitable<IThriftTypeWrapper<TThriftType__>> {
    invariant_violation('Unused test wrapper async API.');
  }

  <<__Override>>
  public static async function genToThrift(
    this $wrapped_value,
  )[zoned_shallow]: Awaitable<int> {
    return $wrapped_value->value;
  }

  <<__Override>>
  public async function genUnwrap()[zoned_shallow]: Awaitable<int> {
    return $this->value;
  }

  <<__Override>>
  public async function genWrap(int $value)[zoned_shallow]: Awaitable<void> {
    $this->value = $value;
  }
}

final class ThriftKeySerializerWrappedTerseStruct implements IThriftStruct {
  use ThriftSerializationTrait;

  const ThriftStructTypes::TSpec SPEC = dict[
    1 => shape(
      'var' => 'wrapped_i64',
      'type' => TType::I64,
      'is_wrapped' => true,
      'is_terse' => true,
    ),
  ];
  const dict<string, int> FIELDMAP = dict[
    'wrapped_i64' => 1,
  ];
  const type TConstructorShape = shape();
  const int STRUCTURAL_ID = 1;

  private ThriftKeySerializerTestI64Wrapper $wrappedI64;

  public function __construct()[] {
    $this->wrappedI64 =
      ThriftKeySerializerTestI64Wrapper::fromThrift_DO_NOT_USE_THRIFT_INTERNAL<
        int,
      >(0);
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function getName()[]: string {
    return 'ThriftKeySerializerWrappedTerseStruct';
  }

  // @lint-ignore HackLint5520 Thrift-generated wrapped accessors use snake_case.
  public function get_wrapped_i64()[]: ThriftKeySerializerTestI64Wrapper {
    return $this->wrappedI64;
  }

  public static function getAllStructuredAnnotations(
  )[write_props]: TStructAnnotations {
    return shape(
      'struct' => dict[],
      'fields' => dict[
        'wrapped_i64' => shape(
          'field' => dict[],
          'type' => dict[],
        ),
      ],
    );
  }

  public function getInstanceKey()[write_props]: string {
    return TCompactSerializer::serialize($this);
  }
}

final class ThriftKeySerializerOuterTerseStruct implements IThriftStruct {
  use ThriftSerializationTrait;

  const ThriftStructTypes::TSpec SPEC = dict[
    1 => shape(
      'var' => 'inner',
      'type' => TType::STRUCT,
      'class' => ThriftKeySerializerWrappedTerseStruct::class,
      'is_terse' => true,
    ),
  ];
  const dict<string, int> FIELDMAP = dict[
    'inner' => 1,
  ];
  const type TConstructorShape = shape();
  const int STRUCTURAL_ID = 2;

  public ThriftKeySerializerWrappedTerseStruct $inner;

  public function __construct()[] {
    $this->inner = ThriftKeySerializerWrappedTerseStruct::withDefaultValues();
  }

  public static function withDefaultValues()[]: this {
    return new static();
  }

  public function getName()[]: string {
    return 'ThriftKeySerializerOuterTerseStruct';
  }

  public static function getAllStructuredAnnotations(
  )[write_props]: TStructAnnotations {
    return shape(
      'struct' => dict[],
      'fields' => dict[
        'inner' => shape(
          'field' => dict[],
          'type' => dict[],
        ),
      ],
    );
  }

  public function getInstanceKey()[write_props]: string {
    return TCompactSerializer::serialize($this);
  }
}
