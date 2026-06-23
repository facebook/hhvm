<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
final class ThriftObjectKeyContainerIntegrationTest extends WWWTest {

  use ClassLevelTest;

  public function testGeneratedObjectKeyContainersAreFunctional(): void {
    $container = self::newObjectKeyContainer();

    expect($container->struct_set)->toBeInstanceOf(ThriftSet::class);
    expect($container->struct_map)->toBeInstanceOf(ThriftMap::class);
    expect($container->bool_set)->toBeInstanceOf(ThriftSet::class);
    expect($container->bool_map)->toBeInstanceOf(ThriftMap::class);
    expect($container->value_set)->toBeInstanceOf(ThriftSet::class);
    expect($container->value_map)->toBeInstanceOf(ThriftMap::class);
    expect(self::describeContainer($container))->toEqual(
      self::expectedDescription(),
    );
  }

  public function testObjectKeyContainersRoundTripThroughNormalSerializers(
  ): void {
    $container = self::newObjectKeyContainer();
    $expected = self::expectedDescription();

    $compact_serialized = TCompactSerializer::serialize($container);
    $compact_deserialized = TCompactSerializer::deserialize(
      $compact_serialized,
      ObjectKeyContainerStruct::withDefaultValues(),
    );
    expect(self::describeContainer($compact_deserialized))->toEqual($expected);

    $binary_serialized = TBinarySerializer::serialize($container);
    $binary_deserialized = TBinarySerializer::deserialize(
      $binary_serialized,
      ObjectKeyContainerStruct::withDefaultValues(),
    );
    expect(self::describeContainer($binary_deserialized))->toEqual($expected);
  }

  public function testObjectKeyContainersRoundTripThroughSimpleJSON(): void {
    $container = self::newObjectKeyContainer();
    $expected = self::expectedDescription();
    $serialized = JSONThriftSerializer::serialize($container);

    $bool_map_serialized = JSONThriftSerializer::serialize(
      ObjectKeyContainerStruct::fromShape(shape(
        'bool_map' => nullthrows($container->bool_map, 'bool_map must be set'),
      )),
    );
    $decoded_bool_map = fb_json_decode($bool_map_serialized);
    expect($decoded_bool_map['bool_map']['true'])->toEqual('yes');
    expect($decoded_bool_map['bool_map']['false'])->toEqual('no');

    $struct_key_serialized = JSONThriftSerializer::serialize(
      ObjectKeyContainerStruct::fromShape(shape(
        'struct_set' =>
          nullthrows($container->struct_set, 'struct_set must be set'),
        'struct_map' =>
          nullthrows($container->struct_map, 'struct_map must be set'),
      )),
    );
    $struct_key_deserialized = JSONThriftSerializer::deserialize(
      $struct_key_serialized,
      ObjectKeyContainerStruct::withDefaultValues(),
    );
    expect(self::describeStructSet($struct_key_deserialized))
      ->toEqual($expected['struct_set']);
    expect(self::describeStructMap($struct_key_deserialized))
      ->toEqual($expected['struct_map']);

    $json_deserialized = JSONThriftSerializer::deserialize(
      $serialized,
      ObjectKeyContainerStruct::withDefaultValues(),
    );
    expect(self::describeContainer($json_deserialized))->toEqual($expected);

    $protocol_deserialized = ObjectKeyContainerStruct::withDefaultValues();
    $protocol_deserialized->read(
      new TSimpleJSONProtocol(new TMemoryBuffer($serialized)),
    );
    expect(self::describeContainer($protocol_deserialized))->toEqual($expected);
  }

  public function testObjectKeyContainersReadLegacyUnquotedSimpleJSONBoolKeys(
  ): void {
    $deserialized = JSONThriftSerializer::deserialize(
      '{"bool_map":{true:"yes",false:"no"}}',
      ObjectKeyContainerStruct::withDefaultValues(),
    );

    expect(self::sortBoolPairsTrueFirst(
      nullthrows($deserialized->bool_map, 'bool_map must be set')->toShape(),
    ))->toEqual(vec[tuple(true, 'yes'), tuple(false, 'no')]);
  }

  public function testObjectKeyContainersRoundTripThroughShape(): void {
    $container = self::newObjectKeyContainer();
    $shape = $container->__toShape();
    $expected = self::expectedDescription();

    expect(nullthrows(Shapes::idx($shape, 'struct_set'), 'struct_set shape'))
      ->toHaveSameValuesInAnyOrderAs(vec[
        self::objectKeyStructShape(1, 'one'),
        self::objectKeyStructShape(2, 'two'),
      ]);
    expect(nullthrows(Shapes::idx($shape, 'struct_map'), 'struct_map shape'))
      ->toHaveSameValuesInAnyOrderAs(vec[
        tuple(self::objectKeyStructShape(1, 'one'), 'first'),
        tuple(self::objectKeyStructShape(2, 'two'), 'second'),
      ]);
    expect(Vec\map(
      nullthrows(Shapes::idx($shape, 'value_set'), 'value_set shape'),
      self::describeValueShape<>,
    ))->toHaveSameValuesInAnyOrderAs($expected['value_set']);
    expect(Vec\map(
      nullthrows(Shapes::idx($shape, 'value_map'), 'value_map shape'),
      $entry ==> tuple(self::describeValueShape($entry[0]), $entry[1]),
    ))->toHaveSameValuesInAnyOrderAs($expected['value_map']);
    expect(self::describeContainerShape($shape))->toEqual($expected);

    $round_tripped = ObjectKeyContainerStruct::__fromShape($shape);
    expect(self::describeContainer($round_tripped))->toEqual($expected);
  }

  public function testObjectKeyContainerFromShapeRejectsDuplicateObjectKeys(
  ): void {
    $set_shape = self::newObjectKeyContainer()->__toShape();
    $set_shape['struct_set'] = vec[
      self::objectKeyStructShape(1, 'one'),
      self::objectKeyStructShape(1, 'one'),
    ];
    expect(() ==> ObjectKeyContainerStruct::__fromShape($set_shape))->toThrow(
      InvariantViolationException::class,
      'Duplicate element',
    );

    $map_shape = self::newObjectKeyContainer()->__toShape();
    $map_shape['struct_map'] = vec[
      tuple(self::objectKeyStructShape(1, 'one'), 'first'),
      tuple(self::objectKeyStructShape(1, 'one'), 'again'),
    ];
    expect(() ==> ObjectKeyContainerStruct::__fromShape($map_shape))->toThrow(
      InvariantViolationException::class,
      'Duplicate key',
    );
  }

  public function testAdaptedObjectKeyContainersRoundTripThroughSerializers(
  ): void {
    $container = self::newAdaptedObjectKeyContainer();
    $expected = self::expectedAdaptedDescription();

    expect(self::describeAdaptedContainer($container))->toEqual($expected);

    $compact_deserialized = TCompactSerializer::deserialize(
      TCompactSerializer::serialize($container),
      ObjectKeyAdaptedContainerStruct::withDefaultValues(),
    );
    expect(self::describeAdaptedContainer($compact_deserialized))
      ->toEqual($expected);

    $binary_deserialized = TBinarySerializer::deserialize(
      TBinarySerializer::serialize($container),
      ObjectKeyAdaptedContainerStruct::withDefaultValues(),
    );
    expect(self::describeAdaptedContainer($binary_deserialized))
      ->toEqual($expected);
  }

  public function testAdaptedObjectKeyContainersRoundTripThroughSimpleJSON(
  ): void {
    $container = self::newAdaptedObjectKeyContainer();
    $expected = self::expectedAdaptedDescription();
    $serialized = JSONThriftSerializer::serialize($container);

    $json_deserialized = JSONThriftSerializer::deserialize(
      $serialized,
      ObjectKeyAdaptedContainerStruct::withDefaultValues(),
    );
    expect(self::describeAdaptedContainer($json_deserialized))
      ->toEqual($expected);

    $protocol_deserialized =
      ObjectKeyAdaptedContainerStruct::withDefaultValues();
    $protocol_deserialized->read(
      new TSimpleJSONProtocol(new TMemoryBuffer($serialized)),
    );
    expect(self::describeAdaptedContainer($protocol_deserialized))
      ->toEqual($expected);
  }

  public function testAdaptedObjectKeyContainersRoundTripThroughShape(): void {
    $container = self::newAdaptedObjectKeyContainer();
    $shape = $container->__toShape();
    $expected = self::expectedAdaptedDescription();

    expect(nullthrows(
      Shapes::idx($shape, 'adapted_struct_map'),
      'adapted_struct_map shape',
    ))->toHaveSameValuesInAnyOrderAs($expected['adapted_struct_map']);
    expect(nullthrows(
      Shapes::idx($shape, 'adapted_struct_set'),
      'adapted_struct_set shape',
    ))->toHaveSameValuesInAnyOrderAs($expected['adapted_struct_set']);
    expect(nullthrows(
      Shapes::idx($shape, 'adapted_i32_map'),
      'adapted_i32_map shape',
    ))->toHaveSameValuesInAnyOrderAs($expected['adapted_i32_map']);
    expect(nullthrows(
      Shapes::idx($shape, 'adapted_i32_set'),
      'adapted_i32_set shape',
    ))->toHaveSameValuesInAnyOrderAs($expected['adapted_i32_set']);
    expect(nullthrows(
      Shapes::idx($shape, 'adapted_struct_value_map'),
      'adapted_struct_value_map shape',
    ))->toHaveSameValuesInAnyOrderAs($expected['adapted_struct_value_map']);
    expect(nullthrows(
      Shapes::idx($shape, 'adapted_struct_key_value_map'),
      'adapted_struct_key_value_map shape',
    ))->toHaveSameValuesInAnyOrderAs($expected['adapted_struct_key_value_map']);
    expect(self::describeAdaptedContainerShape($shape))->toEqual($expected);

    $round_tripped = ObjectKeyAdaptedContainerStruct::__fromShape($shape);
    expect(self::describeAdaptedContainer($round_tripped))->toEqual($expected);
  }

  public function testAdaptedObjectKeyContainerFromShapeRejectsDuplicateKeys(
  ): void {
    $set_shape = self::newAdaptedObjectKeyContainer()->__toShape();
    $set_shape['adapted_i32_set'] = vec['1', '1'];
    expect(() ==> ObjectKeyAdaptedContainerStruct::__fromShape($set_shape))
      ->toThrow(InvariantViolationException::class, 'Duplicate element');

    $map_shape = self::newAdaptedObjectKeyContainer()->__toShape();
    $map_shape['adapted_i32_map'] = vec[
      tuple('1', 'first'),
      tuple('1', 'again'),
    ];
    expect(() ==> ObjectKeyAdaptedContainerStruct::__fromShape($map_shape))
      ->toThrow(InvariantViolationException::class, 'Duplicate key');
  }

  public async function testWrappedObjectKeyMapValuesRoundTrip(
  ): Awaitable<void> {
    $container = await self::genWrappedValueObjectKeyContainer();
    $expected = self::expectedWrappedValueDescription();

    expect(await self::genDescribeWrappedValueContainer($container))
      ->toEqual($expected);

    $compact_deserialized = TCompactSerializer::deserialize(
      TCompactSerializer::serialize($container),
      ObjectKeyWrappedValueContainerStruct::withDefaultValues(),
    );
    expect(await self::genDescribeWrappedValueContainer($compact_deserialized))
      ->toEqual($expected);

    $binary_deserialized = TBinarySerializer::deserialize(
      TBinarySerializer::serialize($container),
      ObjectKeyWrappedValueContainerStruct::withDefaultValues(),
    );
    expect(await self::genDescribeWrappedValueContainer($binary_deserialized))
      ->toEqual($expected);

    $shape = await $container->__genToShape();
    expect(self::describeWrappedValueContainerShape($shape))
      ->toEqual($expected);

    $shape_round_tripped =
      await ObjectKeyWrappedValueContainerStruct::__genFromShape($shape);
    expect(await self::genDescribeWrappedValueContainer($shape_round_tripped))
      ->toEqual($expected);
  }

  public async function testWrappedObjectKeyMapValuesRoundTripThroughSimpleJSON(
  ): Awaitable<void> {
    $container = await self::genWrappedValueObjectKeyContainer();
    $expected = self::expectedWrappedValueDescription();
    $serialized = JSONThriftSerializer::serialize($container);

    $json_deserialized = JSONThriftSerializer::deserialize(
      $serialized,
      ObjectKeyWrappedValueContainerStruct::withDefaultValues(),
    );
    expect(await self::genDescribeWrappedValueContainer($json_deserialized))
      ->toEqual($expected);

    $protocol_deserialized =
      ObjectKeyWrappedValueContainerStruct::withDefaultValues();
    $protocol_deserialized->read(
      new TSimpleJSONProtocol(new TMemoryBuffer($serialized)),
    );
    expect(await self::genDescribeWrappedValueContainer($protocol_deserialized))
      ->toEqual($expected);
  }

  private static function newObjectKeyContainer(): ObjectKeyContainerStruct {
    $struct_set = ThriftSet::forStruct<ObjectKeyStruct>();
    $struct_set
      ->add(self::newObjectKeyStruct(2, 'two'))
      ->add(self::newObjectKeyStruct(1, 'one'));

    $struct_map = ThriftMap::forStruct<ObjectKeyStruct, string>();
    $struct_map
      ->set(self::newObjectKeyStruct(2, 'two'), 'second')
      ->set(self::newObjectKeyStruct(1, 'one'), 'first');

    $float_set = ThriftSet::forFloat();
    $float_set->add(2.5)->add(1.25);

    $bool_set = ThriftSet::forBool();
    $bool_set->add(true)->add(false);

    $bool_map = ThriftMap::forBool<string>();
    $bool_map->set(true, 'yes')->set(false, 'no');

    $nested_set = ThriftSet::forStruct<ObjectKeyValue>();
    $nested_set
      ->add(self::newValueWithString('nested'))
      ->add(self::newValueWithInt(3));

    $value_set = ThriftSet::forStruct<ObjectKeyValue>();
    $value_set
      ->add(self::newValueWithString('alpha'))
      ->add(self::newValueWithInt(7))
      ->add(ObjectKeyValue::fromShape(shape('set_value' => $nested_set)));

    $value_map = ThriftMap::forStruct<ObjectKeyValue, int>();
    $value_map
      ->set(self::newValueWithStruct(9, 'nine'), 90)
      ->set(self::newValueWithInt(1), 10);

    return ObjectKeyContainerStruct::fromShape(shape(
      'struct_set' => $struct_set,
      'struct_map' => $struct_map,
      'float_set' => $float_set,
      'bool_set' => $bool_set,
      'bool_map' => $bool_map,
      'value_set' => $value_set,
      'value_map' => $value_map,
      'normal_field' => 'normal',
      'normal_map' => Map {'second' => 2, 'first' => 1},
    ));
  }

  private static function newAdaptedObjectKeyContainer(
  ): ObjectKeyAdaptedContainerStruct {
    $adapted_struct_map =
      new ThriftMap<ObjectKeyStructShapeAdapter::THackType, string>(
        TType::STRUCT,
        Shapes::at(ObjectKeyAdaptedContainerStruct::SPEC[1], 'key'),
      );
    $adapted_struct_map
      ->set(shape('id' => 2, 'name' => 'two'), 'second')
      ->set(shape('id' => 1, 'name' => 'one'), 'first');

    $adapted_struct_set = new ThriftSet<ObjectKeyStructShapeAdapter::THackType>(
      TType::STRUCT,
      Shapes::at(ObjectKeyAdaptedContainerStruct::SPEC[2], 'elem'),
    );
    $adapted_struct_set
      ->add(shape('id' => 2, 'name' => 'two'))
      ->add(shape('id' => 1, 'name' => 'one'));

    $adapted_i32_map = new ThriftMap<AdapterTestIntToString::THackType, string>(
      TType::I32,
      Shapes::at(ObjectKeyAdaptedContainerStruct::SPEC[3], 'key'),
    );
    $adapted_i32_map->set('2', 'second')->set('1', 'first');

    $adapted_i32_set = new ThriftSet<AdapterTestIntToString::THackType>(
      TType::I32,
      Shapes::at(ObjectKeyAdaptedContainerStruct::SPEC[4], 'elem'),
    );
    $adapted_i32_set->add('2')->add('1');

    $adapted_struct_value_map = ThriftMap::forStruct<
      ObjectKeyStruct,
      ObjectKeyStructShapeAdapter::THackType,
    >();
    $adapted_struct_value_map
      ->set(
        self::newObjectKeyStruct(1, 'one'),
        shape('id' => 10, 'name' => 'ten'),
      )
      ->set(
        self::newObjectKeyStruct(2, 'two'),
        shape('id' => 20, 'name' => 'twenty'),
      );

    $adapted_struct_key_value_map = new ThriftMap<
      ObjectKeyStructShapeAdapter::THackType,
      ObjectKeyStructShapeAdapter::THackType,
    >(
      TType::STRUCT,
      Shapes::at(ObjectKeyAdaptedContainerStruct::SPEC[6], 'key'),
    );
    $adapted_struct_key_value_map
      ->set(
        shape('id' => 1, 'name' => 'one'),
        shape('id' => 10, 'name' => 'ten'),
      )
      ->set(
        shape('id' => 2, 'name' => 'two'),
        shape('id' => 20, 'name' => 'twenty'),
      );

    return ObjectKeyAdaptedContainerStruct::fromShape(shape(
      'adapted_struct_map' => $adapted_struct_map,
      'adapted_struct_set' => $adapted_struct_set,
      'adapted_i32_map' => $adapted_i32_map,
      'adapted_i32_set' => $adapted_i32_set,
      'adapted_struct_value_map' => $adapted_struct_value_map,
      'adapted_struct_key_value_map' => $adapted_struct_key_value_map,
    ));
  }

  private static async function genWrappedValueObjectKeyContainer(
  ): Awaitable<ObjectKeyWrappedValueContainerStruct> {
    $wrapped_value_map = new ThriftMap<
      ObjectKeyStruct,
      thrift_adapted_types\ObjectKeyWrappedValueStruct,
    >(
      TType::STRUCT,
      Shapes::at(ObjectKeyWrappedValueContainerStruct::SPEC[1], 'key'),
    );
    $wrapped_value_map
      ->set(
        self::newObjectKeyStruct(1, 'one'),
        thrift_adapted_types\ObjectKeyWrappedValueStruct::fromShape(
          shape('id' => 10, 'name' => 'ten'),
        ),
      )
      ->set(
        self::newObjectKeyStruct(2, 'two'),
        thrift_adapted_types\ObjectKeyWrappedValueStruct::fromShape(
          shape('id' => 20, 'name' => 'twenty'),
        ),
      );

    return await ObjectKeyWrappedValueContainerStruct::genFromShape(shape(
      'wrapped_struct_value_map' => $wrapped_value_map,
    ));
  }

  private static function expectedDescription(): dict<string, mixed> {
    return dict[
      'struct_set' => vec['1:one', '2:two'],
      'struct_map' => vec[
        tuple('1:one', 'first'),
        tuple('2:two', 'second'),
      ],
      'float_set' => vec[1.25, 2.5],
      'bool_set' => vec[true, false],
      'bool_map' => vec[
        tuple(true, 'yes'),
        tuple(false, 'no'),
      ],
      'value_set' => vec[
        'int:7',
        'set:{int:3,string:nested}',
        'string:alpha',
      ],
      'value_map' => vec[
        tuple('int:1', 10),
        tuple('struct:9:nine', 90),
      ],
      'normal_field' => 'normal',
      'normal_map' => dict[
        'first' => 1,
        'second' => 2,
      ],
    ];
  }

  private static function expectedAdaptedDescription(): dict<string, mixed> {
    return dict[
      'adapted_struct_map' => vec[
        tuple(shape('id' => 1, 'name' => 'one'), 'first'),
        tuple(shape('id' => 2, 'name' => 'two'), 'second'),
      ],
      'adapted_struct_set' => vec[
        shape('id' => 1, 'name' => 'one'),
        shape('id' => 2, 'name' => 'two'),
      ],
      'adapted_i32_map' => vec[tuple('1', 'first'), tuple('2', 'second')],
      'adapted_i32_set' => vec['1', '2'],
      'adapted_struct_value_map' => vec[
        tuple(
          self::objectKeyStructShape(1, 'one'),
          shape('id' => 10, 'name' => 'ten'),
        ),
        tuple(
          self::objectKeyStructShape(2, 'two'),
          shape('id' => 20, 'name' => 'twenty'),
        ),
      ],
      'adapted_struct_key_value_map' => vec[
        tuple(
          self::objectKeyStructShape(1, 'one'),
          shape('id' => 10, 'name' => 'ten'),
        ),
        tuple(
          self::objectKeyStructShape(2, 'two'),
          shape('id' => 20, 'name' => 'twenty'),
        ),
      ],
    ];
  }

  private static function expectedWrappedValueDescription(
  ): vec<(string, string)> {
    return vec[
      tuple('1:one', '10:ten'),
      tuple('2:two', '20:twenty'),
    ];
  }

  private static function describeContainerShape(
    ObjectKeyContainerStruct::TShape $shape,
  ): dict<string, mixed> {
    return dict[
      'struct_set' => self::sortStrings(
        Vec\map(
          nullthrows(Shapes::idx($shape, 'struct_set'), 'struct_set shape'),
          self::describeObjectKeyStructShape<>,
        ),
      ),
      'struct_map' => self::sortStringPairs(
        Vec\map(
          nullthrows(Shapes::idx($shape, 'struct_map'), 'struct_map shape'),
          $entry ==>
            tuple(self::describeObjectKeyStructShape($entry[0]), $entry[1]),
        ),
      ),
      'float_set' => Vec\sort(
        nullthrows(Shapes::idx($shape, 'float_set'), 'float_set shape'),
      ),
      'bool_set' => self::sortBoolsTrueFirst(
        nullthrows(Shapes::idx($shape, 'bool_set'), 'bool_set shape'),
      ),
      'bool_map' => self::sortBoolPairsTrueFirst(
        nullthrows(Shapes::idx($shape, 'bool_map'), 'bool_map shape'),
      ),
      'value_set' => self::sortStrings(
        Vec\map(
          nullthrows(Shapes::idx($shape, 'value_set'), 'value_set shape'),
          self::describeValueShape<>,
        ),
      ),
      'value_map' => self::sortStringPairs(
        Vec\map(
          nullthrows(Shapes::idx($shape, 'value_map'), 'value_map shape'),
          $entry ==> tuple(self::describeValueShape($entry[0]), $entry[1]),
        ),
      ),
      'normal_field' => $shape['normal_field'],
      'normal_map' => Dict\sort_by_key($shape['normal_map']),
    ];
  }

  private static function describeAdaptedContainerShape(
    ObjectKeyAdaptedContainerStruct::TShape $shape,
  ): dict<string, mixed> {
    return dict[
      'adapted_struct_map' => self::sortShapePairsById(
        nullthrows(
          Shapes::idx($shape, 'adapted_struct_map'),
          'adapted_struct_map shape',
        ),
      ),
      'adapted_struct_set' => self::sortObjectKeyStructShapes(
        nullthrows(
          Shapes::idx($shape, 'adapted_struct_set'),
          'adapted_struct_set shape',
        ),
      ),
      'adapted_i32_map' => self::sortStringPairs(
        nullthrows(
          Shapes::idx($shape, 'adapted_i32_map'),
          'adapted_i32_map shape',
        ),
      ),
      'adapted_i32_set' => self::sortStrings(
        nullthrows(
          Shapes::idx($shape, 'adapted_i32_set'),
          'adapted_i32_set shape',
        ),
      ),
      'adapted_struct_value_map' => self::sortShapePairsById(
        nullthrows(
          Shapes::idx($shape, 'adapted_struct_value_map'),
          'adapted_struct_value_map shape',
        ),
      ),
      'adapted_struct_key_value_map' => self::sortShapePairsById(
        nullthrows(
          Shapes::idx($shape, 'adapted_struct_key_value_map'),
          'adapted_struct_key_value_map shape',
        ),
      ),
    ];
  }

  private static function describeWrappedValueContainerShape(
    ObjectKeyWrappedValueContainerStruct::TShape $shape,
  ): vec<(string, string)> {
    return self::sortStringPairs(
      Vec\map(
        nullthrows(
          Shapes::idx($shape, 'wrapped_struct_value_map'),
          'wrapped_struct_value_map shape',
        ),
        $entry ==> tuple(
          self::describeObjectKeyStructShape($entry[0]),
          self::describeObjectKeyWrappedValueStructShape($entry[1]),
        ),
      ),
    );
  }

  private static function describeAdaptedContainer(
    ObjectKeyAdaptedContainerStruct $container,
  ): dict<string, mixed> {
    return dict[
      'adapted_struct_map' => self::sortShapePairsById(
        nullthrows(
          $container->adapted_struct_map,
          'adapted_struct_map must be set',
        )->toShape(),
      ),
      'adapted_struct_set' => self::sortObjectKeyStructShapes(
        nullthrows(
          $container->adapted_struct_set,
          'adapted_struct_set must be set',
        )->toVec(),
      ),
      'adapted_i32_map' => self::sortStringPairs(
        nullthrows($container->adapted_i32_map, 'adapted_i32_map must be set')
          ->toShape(),
      ),
      'adapted_i32_set' => self::sortStrings(
        nullthrows($container->adapted_i32_set, 'adapted_i32_set must be set')
          ->toVec(),
      ),
      'adapted_struct_value_map' => self::sortShapePairsById(
        Vec\map(
          nullthrows(
            $container->adapted_struct_value_map,
            'adapted_struct_value_map must be set',
          )->toShape(),
          $entry ==>
            tuple(self::objectKeyStructShapeFromStruct($entry[0]), $entry[1]),
        ),
      ),
      'adapted_struct_key_value_map' => self::sortShapePairsById(
        nullthrows(
          $container->adapted_struct_key_value_map,
          'adapted_struct_key_value_map must be set',
        )->toShape(),
      ),
    ];
  }

  private static async function genDescribeWrappedValueContainer(
    ObjectKeyWrappedValueContainerStruct $container,
  ): Awaitable<vec<(string, string)>> {
    $entries = await Vec\map_async(
      nullthrows(
        $container->wrapped_struct_value_map,
        'wrapped_struct_value_map must be set',
      )->toShape(),
      async $entry ==> {
        list($key, $wrapped_value) = $entry;
        $value = await $wrapped_value->genUnwrap();
        return tuple(
          self::describeObjectKeyStruct($key),
          self::describeObjectKeyWrappedValueStruct($value),
        );
      },
    );
    return self::sortStringPairs($entries);
  }

  private static function describeContainer(
    ObjectKeyContainerStruct $container,
  ): dict<string, mixed> {
    return dict[
      'struct_set' => self::describeStructSet($container),
      'struct_map' => self::describeStructMap($container),
      'float_set' => Vec\sort(
        nullthrows($container->float_set, 'float_set must be set')->toVec(),
      ),
      'bool_set' => self::sortBoolsTrueFirst(
        nullthrows($container->bool_set, 'bool_set must be set')->toVec(),
      ),
      'bool_map' => self::sortBoolPairsTrueFirst(
        nullthrows($container->bool_map, 'bool_map must be set')->toShape(),
      ),
      'value_set' => self::sortStrings(
        Vec\map(
          nullthrows($container->value_set, 'value_set must be set')->toVec(),
          self::describeValue<>,
        ),
      ),
      'value_map' => self::sortStringPairs(
        Vec\map(
          nullthrows($container->value_map, 'value_map must be set')->toShape(),
          $entry ==> tuple(self::describeValue($entry[0]), $entry[1]),
        ),
      ),
      'normal_field' => $container->normal_field,
      'normal_map' => Dict\sort_by_key(dict($container->normal_map)),
    ];
  }

  private static function describeStructSet(
    ObjectKeyContainerStruct $container,
  ): vec<string> {
    return self::sortStrings(
      Vec\map(
        nullthrows($container->struct_set, 'struct_set must be set')->toVec(),
        self::describeObjectKeyStruct<>,
      ),
    );
  }

  private static function describeStructMap(
    ObjectKeyContainerStruct $container,
  ): vec<(string, string)> {
    return self::sortStringPairs(
      Vec\map(
        nullthrows($container->struct_map, 'struct_map must be set')->toShape(),
        $entry ==> tuple(self::describeObjectKeyStruct($entry[0]), $entry[1]),
      ),
    );
  }

  private static function sortStrings(vec<string> $values): vec<string> {
    return Vec\sort($values);
  }

  private static function sortStringPairs<Tv>(
    vec<(string, Tv)> $entries,
  ): vec<(string, Tv)> {
    return Vec\sort($entries, ($a, $b) ==> Str\compare($a[0], $b[0]));
  }

  private static function sortBoolsTrueFirst(vec<bool> $values): vec<bool> {
    return Vec\sort($values, ($a, $b) ==> $a === $b ? 0 : ($a ? -1 : 1));
  }

  private static function sortBoolPairsTrueFirst<Tv>(
    vec<(bool, Tv)> $entries,
  ): vec<(bool, Tv)> {
    return
      Vec\sort($entries, ($a, $b) ==> $a[0] === $b[0] ? 0 : ($a[0] ? -1 : 1));
  }

  private static function sortObjectKeyStructShapes(
    vec<ObjectKeyStructShapeAdapter::THackType> $values,
  ): vec<ObjectKeyStructShapeAdapter::THackType> {
    return Vec\sort($values, ($a, $b) ==> self::compareIds($a['id'], $b['id']));
  }

  private static function sortShapePairsById<Tv>(
    vec<(ObjectKeyStructShapeAdapter::THackType, Tv)> $entries,
  ): vec<(ObjectKeyStructShapeAdapter::THackType, Tv)> {
    return Vec\sort(
      $entries,
      ($a, $b) ==> self::compareIds($a[0]['id'], $b[0]['id']),
    );
  }

  private static function compareIds(int $a, int $b): int {
    if ($a === $b) {
      return 0;
    }
    return $a < $b ? -1 : 1;
  }

  private static function newObjectKeyStruct(
    int $id,
    string $name,
  ): ObjectKeyStruct {
    return ObjectKeyStruct::fromShape(shape('id' => $id, 'name' => $name));
  }

  private static function objectKeyStructShape(
    int $id,
    string $name,
  ): ObjectKeyStructShapeAdapter::THackType {
    return shape('id' => $id, 'name' => $name);
  }

  private static function objectKeyStructShapeFromStruct(
    ObjectKeyStruct $struct,
  ): ObjectKeyStructShapeAdapter::THackType {
    return self::objectKeyStructShape($struct->id, $struct->name);
  }

  private static function describeObjectKeyStructShape(
    ObjectKeyStruct::TShape $shape,
  ): string {
    return Str\format('%d:%s', $shape['id'], $shape['name']);
  }

  private static function describeObjectKeyWrappedValueStructShape(
    thrift_adapted_types\ObjectKeyWrappedValueStruct::TShape $shape,
  ): string {
    return Str\format('%d:%s', $shape['id'], $shape['name']);
  }

  private static function describeValueShape(
    ObjectKeyValue::TShape $shape,
  ): string {
    return self::describeValue(ObjectKeyValue::__fromShape($shape));
  }

  private static function newValueWithInt(int $value): ObjectKeyValue {
    return ObjectKeyValue::fromShape(shape('int_value' => $value));
  }

  private static function newValueWithString(string $value): ObjectKeyValue {
    return ObjectKeyValue::fromShape(shape('string_value' => $value));
  }

  private static function newValueWithStruct(
    int $id,
    string $name,
  ): ObjectKeyValue {
    return ObjectKeyValue::fromShape(
      shape('struct_value' => self::newObjectKeyStruct($id, $name)),
    );
  }

  private static function describeObjectKeyStruct(
    ObjectKeyStruct $struct,
  ): string {
    return Str\format('%d:%s', $struct->id, $struct->name);
  }

  private static function describeObjectKeyWrappedValueStruct(
    thrift_adapted_types\ObjectKeyWrappedValueStruct $struct,
  ): string {
    return Str\format('%d:%s', $struct->id, $struct->name);
  }

  private static function describeValue(ObjectKeyValue $value): string {
    switch ($value->getType()) {
      case ObjectKeyValueEnum::bool_value:
        return $value->getx_bool_value() ? 'bool:true' : 'bool:false';
      case ObjectKeyValueEnum::int_value:
        return Str\format('int:%d', $value->getx_int_value());
      case ObjectKeyValueEnum::string_value:
        return 'string:'.$value->getx_string_value();
      case ObjectKeyValueEnum::struct_value:
        return
          'struct:'.self::describeObjectKeyStruct($value->getx_struct_value());
      case ObjectKeyValueEnum::list_value:
        return 'list:['.
          Str\join(
            Vec\map($value->getx_list_value(), self::describeValue<>),
            ',',
          ).
          ']';
      case ObjectKeyValueEnum::set_value:
        return 'set:{'.
          Str\join(
            self::sortStrings(
              Vec\map($value->getx_set_value()->toVec(), self::describeValue<>),
            ),
            ',',
          ).
          '}';
      case ObjectKeyValueEnum::map_value:
        return 'map:{'.
          Str\join(
            self::sortStrings(
              Vec\map(
                $value->getx_map_value()->toShape(),
                $entry ==> self::describeValue($entry[0]).
                  '=>'.
                  self::describeValue($entry[1]),
              ),
            ),
            ',',
          ).
          '}';
      case ObjectKeyValueEnum::_EMPTY_:
        return 'empty';
    }
  }
}

final class ObjectKeyStructShapeAdapter implements IThriftAdapter {
  const type TThriftType = ObjectKeyStruct;
  const type THackType = shape('id' => int, 'name' => string);

  public static function fromThrift(
    ObjectKeyStruct $thrift_value,
  )[]: self::THackType {
    return shape('id' => $thrift_value->id, 'name' => $thrift_value->name);
  }

  public static function toThrift(
    self::THackType $hack_value,
  )[]: ObjectKeyStruct {
    return ObjectKeyStruct::fromShape($hack_value);
  }
}
