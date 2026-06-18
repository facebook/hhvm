<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
final class ThriftMapTest extends WWWTest {

  private function getStructKeySpec(): ThriftStructTypes::TGenericSpec {
    return shape(
      'type' => TType::STRUCT,
      'class' => SimpleStruct::class,
    );
  }

  private function createMap(): ThriftMap<SimpleStruct, string> {
    return new ThriftMap<SimpleStruct, string>(
      TType::STRUCT,
      $this->getStructKeySpec(),
    );
  }

  public function testSetAndGet(): void {
    $map = $this->createMap();
    $key = SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));
    $map->set($key, 'world');

    expect($map->get($key))->toEqual('world');
    expect($map->count())->toEqual(1);
  }

  public function testValueBasedIdentity(): void {
    $map = $this->createMap();
    $key1 =
      SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));
    $key2 =
      SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));

    $map->set($key1, 'first');
    $map->set($key2, 'second');

    // Same struct values = same key, so second overwrites first
    expect($map->count())->toEqual(1);
    expect($map->get($key1))->toEqual('second');
  }

  public function testDifferentKeysAreDifferent(): void {
    $map = $this->createMap();
    $key1 = SimpleStruct::fromShape(shape('a_i32' => 1));
    $key2 = SimpleStruct::fromShape(shape('a_i32' => 2));

    $map->set($key1, 'one');
    $map->set($key2, 'two');

    expect($map->count())->toEqual(2);
    expect($map->get($key1))->toEqual('one');
    expect($map->get($key2))->toEqual('two');
  }

  public function testContains(): void {
    $map = $this->createMap();
    $key = SimpleStruct::fromShape(shape('a_i32' => 42));
    $other = SimpleStruct::fromShape(shape('a_i32' => 99));

    $map->set($key, 'val');

    expect($map->contains($key))->toBeTrue();
    expect($map->contains($other))->toBeFalse();
  }

  public function testAt(): void {
    $map = $this->createMap();
    $key = SimpleStruct::fromShape(shape('a_i32' => 42));
    $map->set($key, 'val');

    expect($map->at($key))->toEqual('val');
  }

  public function testMutableValuesAreShared(): void {
    $map = new ThriftMap<SimpleStruct, SimpleStruct>(
      TType::STRUCT,
      $this->getStructKeySpec(),
    );
    $key = SimpleStruct::fromShape(shape('a_i32' => 42));
    $value = SimpleStruct::fromShape(shape('a_i32' => 1));

    $map->set($key, $value);
    $value->a_i32 = 2;
    expect($map->at($key)->a_i32)->toEqual(2);

    $returned = $map->at($key);
    $returned->a_i32 = 3;
    expect($map->at($key)->a_i32)->toEqual(3);
  }

  public function testRemove(): void {
    $map = $this->createMap();
    $key = SimpleStruct::fromShape(shape('a_i32' => 42));
    $map->set($key, 'val');

    expect($map->contains($key))->toBeTrue();
    $map->remove($key);
    expect($map->contains($key))->toBeFalse();
    expect($map->count())->toEqual(0);
  }

  public function testRemoveMissingKeyIsNoop(): void {
    $map = $this->createMap();
    $present = SimpleStruct::fromShape(shape('a_i32' => 1));
    $missing = SimpleStruct::fromShape(shape('a_i32' => 2));
    $map->set($present, 'val');

    $map->remove($missing);

    expect($map->count())->toEqual(1);
    expect($map->contains($present))->toBeTrue();
  }

  public function testIsEmpty(): void {
    $map = $this->createMap();
    expect($map->isEmpty())->toBeTrue();

    $key = SimpleStruct::fromShape(shape('a_i32' => 42));
    $map->set($key, 'val');
    expect($map->isEmpty())->toBeFalse();
  }

  public function testForeach(): void {
    $map = $this->createMap();
    $key1 = SimpleStruct::fromShape(shape('a_i32' => 1));
    $key2 = SimpleStruct::fromShape(shape('a_i32' => 2));

    $map->set($key1, 'one');
    $map->set($key2, 'two');

    $items = vec[];
    foreach ($map as $k => $v) {
      $items[] = tuple($k->a_i32, $v);
    }

    expect($items)->toHaveSameValuesInAnyOrderAs(
      vec[tuple(1, 'one'), tuple(2, 'two')],
    );
  }

  public function testForeachCopiesKeys(): void {
    $map = $this->createMap();
    $key = SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));
    $map->set($key, 'val');

    foreach ($map as $k => $_v) {
      $k->a_i32 = 999;
    }

    foreach ($map as $k => $v) {
      expect($k->a_i32)->toEqual(42);
      expect($v)->toEqual('val');
    }
  }

  public function testMutableCollectionKeysAreCopied(): void {
    $nested_list_spec = shape(
      'type' => TType::LST,
      'etype' => TType::I32,
      'elem' => shape('type' => TType::I32),
      'format' => 'collection',
    );
    $list_key_spec = shape(
      'type' => TType::LST,
      'etype' => TType::LST,
      'elem' => $nested_list_spec,
      'format' => 'collection',
    );
    $map =
      new ThriftMap<Vector<Vector<int>>, string>(TType::LST, $list_key_spec);
    $nested_key = Vector {1, 2};
    $key = Vector {$nested_key};

    $map->set($key, 'original');
    $nested_key[] = 3;

    expect($map->contains(Vector {Vector {1, 2}}))->toBeTrue();
    expect($map->contains($key))->toBeFalse();
    expect($map->get(Vector {Vector {1, 2}}))->toEqual('original');
  }

  public function testMapKeyUsesSerializedContents(): void {
    $map_key_spec = shape(
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::STRING,
      'key' => shape('type' => TType::I32),
      'val' => shape('type' => TType::STRING),
      'format' => 'collection',
    );
    $map = new ThriftMap<Map<int, string>, string>(TType::MAP, $map_key_spec);
    $key = Map {2 => 'two', 1 => 'one'};

    $map->set($key, 'original');
    $key[3] = 'three';

    $equivalent_key = Map {1 => 'one', 2 => 'two'};
    expect($map->contains($equivalent_key))->toBeTrue();
    expect($map->contains($key))->toBeFalse();
    expect($map->get($equivalent_key))->toEqual('original');
  }

  public function testSetKeyUsesSerializedContents(): void {
    $set_key_spec = shape(
      'type' => TType::SET,
      'etype' => TType::I32,
      'elem' => shape('type' => TType::I32),
      'format' => 'collection',
    );
    $map = new ThriftMap<Set<int>, string>(TType::SET, $set_key_spec);
    $key = Set {2, 1};

    $map->set($key, 'original');
    $key[] = 3;

    $equivalent_key = Set {1, 2};
    expect($map->contains($equivalent_key))->toBeTrue();
    expect($map->contains($key))->toBeFalse();
    expect($map->get($equivalent_key))->toEqual('original');
  }

  public function testObjectKeyMapKeyUsesSerializedContents(): void {
    $struct_key_spec = $this->getStructKeySpec();
    $object_key_map_spec =
      HH\FIXME\UNSAFE_CAST<mixed, ThriftStructTypes::TGenericSpec>(
        shape(
          'type' => TType::MAP,
          'ktype' => TType::STRUCT,
          'vtype' => TType::STRING,
          'key' => $struct_key_spec,
          'val' => shape('type' => TType::STRING),
          'format' => 'object_key',
        ),
        'Nested object-key map spec matches generated shape.',
      );
    $map = new ThriftMap<ThriftMap<SimpleStruct, string>, string>(
      TType::MAP,
      $object_key_map_spec,
    );
    $key = $this->createMap();
    $key->set(SimpleStruct::fromShape(shape('a_i32' => 2)), 'two');
    $key->set(SimpleStruct::fromShape(shape('a_i32' => 1)), 'one');

    $map->set($key, 'original');
    $key->set(SimpleStruct::fromShape(shape('a_i32' => 3)), 'three');

    $equivalent_key = $this->createMap();
    $equivalent_key->set(SimpleStruct::fromShape(shape('a_i32' => 1)), 'one');
    $equivalent_key->set(SimpleStruct::fromShape(shape('a_i32' => 2)), 'two');
    expect($map->contains($equivalent_key))->toBeTrue();
    expect($map->contains($key))->toBeFalse();
    expect($map->get($equivalent_key))->toEqual('original');
  }

  public function testStructKeysAreDeepCopied(): void {
    $map = new ThriftMap<ComplexStruct, string>(
      TType::STRUCT,
      shape('type' => TType::STRUCT, 'class' => ComplexStruct::class),
    );
    $key = ComplexStruct::fromShape(
      shape('simple_struct_list' => Vector {self::newSimpleStructWithInt(1)}),
    );

    $map->set($key, 'original');
    $key->simple_struct_list[] = self::newSimpleStructWithInt(2);

    $original_key = ComplexStruct::fromShape(
      shape('simple_struct_list' => Vector {self::newSimpleStructWithInt(1)}),
    );
    expect($map->contains($original_key))->toBeTrue();
    expect($map->contains($key))->toBeFalse();
    expect($map->get($original_key))->toEqual('original');
  }

  public function testWithBoolKeys(): void {
    $bool_spec = shape('type' => TType::BOOL);
    $map = new ThriftMap<bool, string>(TType::BOOL, $bool_spec);

    $map->set(true, 'yes');
    $map->set(false, 'no');

    expect($map->count())->toEqual(2);
    expect($map->get(true))->toEqual('yes');
    expect($map->get(false))->toEqual('no');
  }

  public function testWithFloatKeys(): void {
    $float_spec = shape('type' => TType::DOUBLE);
    $map = new ThriftMap<float, string>(TType::DOUBLE, $float_spec);

    $map->set(1.5, 'one-point-five');
    $map->set(2.5, 'two-point-five');

    expect($map->count())->toEqual(2);
    expect($map->get(1.5))->toEqual('one-point-five');
    expect($map->get(2.5))->toEqual('two-point-five');
  }

  public function testWithUnionKeys(): void {
    $union_spec = shape(
      'type' => TType::STRUCT,
      'class' => SimpleUnion::class,
    );
    $map = new ThriftMap<SimpleUnion, string>(TType::STRUCT, $union_spec);

    $u1 = SimpleUnion::fromShape(shape('a_bool' => true));
    $u2 = SimpleUnion::fromShape(shape('a_byte' => 5));

    $map->set($u1, 'bool-key');
    $map->set($u2, 'byte-key');

    expect($map->count())->toEqual(2);
    expect($map->get($u1))->toEqual('bool-key');
    expect($map->get($u2))->toEqual('byte-key');
  }

  public function testCountable(): void {
    $map = $this->createMap();
    expect(PHP\count($map))->toEqual(0);

    $key = SimpleStruct::fromShape(shape('a_i32' => 42));
    $map->set($key, 'val');
    expect(PHP\count($map))->toEqual(1);
  }

  public function testForStruct(): void {
    $map = ThriftMap::forStruct<SimpleStruct, string>();
    $key = SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));
    $map->set($key, 'world');

    expect($map->get($key))->toEqual('world');
    expect($map->count())->toEqual(1);
  }

  public function testForBool(): void {
    $map = ThriftMap::forBool<string>();

    $map->set(true, 'yes');
    $map->set(false, 'no');

    expect($map->count())->toEqual(2);
    expect($map->get(true))->toEqual('yes');
    expect($map->get(false))->toEqual('no');
  }

  public function testForFloat(): void {
    $map = ThriftMap::forFloat<string>();

    $map->set(1.5, 'one-point-five');
    $map->set(2.5, 'two-point-five');

    expect($map->count())->toEqual(2);
    expect($map->get(1.5))->toEqual('one-point-five');
    expect($map->get(2.5))->toEqual('two-point-five');
  }

  public function testToShape(): void {
    $map = $this->createMap();
    $key1 = SimpleStruct::fromShape(shape('a_i32' => 1));
    $key2 = SimpleStruct::fromShape(shape('a_i32' => 2));

    $map->set($key1, 'one');
    $map->set($key2, 'two');

    $shape = $map->toShape();
    $pairs = Vec\map($shape, $entry ==> tuple($entry[0]->a_i32, $entry[1]));

    expect($pairs)->toHaveSameValuesInAnyOrderAs(
      vec[tuple(1, 'one'), tuple(2, 'two')],
    );
  }

  public function testFromShape(): void {
    $key1 = SimpleStruct::fromShape(shape('a_i32' => 10));
    $key2 = SimpleStruct::fromShape(shape('a_i32' => 20));
    $entries = vec[tuple($key1, 'ten'), tuple($key2, 'twenty')];

    $map = ThriftMap::fromShape<SimpleStruct, string>(
      $entries,
      TType::STRUCT,
      $this->getStructKeySpec(),
    );

    expect($map->count())->toEqual(2);
    expect($map->get($key1))->toEqual('ten');
    expect($map->get($key2))->toEqual('twenty');
  }

  public function testFromShapeRejectsDuplicateKeys(): void {
    $key1 = SimpleStruct::fromShape(shape('a_i32' => 10));
    $key2 = SimpleStruct::fromShape(shape('a_i32' => 10));

    expect(
      () ==> ThriftMap::fromShape<SimpleStruct, string>(
        vec[tuple($key1, 'ten'), tuple($key2, 'also-ten')],
        TType::STRUCT,
        $this->getStructKeySpec(),
      ),
    )->toThrow(InvariantViolationException::class, 'Duplicate key');
  }

  public function testToShapeFromShapeRoundTrip(): void {
    $map = $this->createMap();
    $key1 = SimpleStruct::fromShape(shape('a_i32' => 1, 'a_string' => 'hello'));
    $key2 = SimpleStruct::fromShape(shape('a_i32' => 2, 'a_string' => 'world'));

    $map->set($key1, 'one');
    $map->set($key2, 'two');

    $restored = ThriftMap::fromShape<SimpleStruct, string>(
      $map->toShape(),
      TType::STRUCT,
      $this->getStructKeySpec(),
    );

    expect($restored->count())->toEqual(2);
    expect($restored->get($key1))->toEqual('one');
    expect($restored->get($key2))->toEqual('two');
  }

  public function testToShapeWithBoolKeys(): void {
    $map = ThriftMap::forBool<string>();
    $map->set(true, 'yes');
    $map->set(false, 'no');

    $shape = $map->toShape();

    expect($shape)->toHaveSameValuesInAnyOrderAs(
      vec[tuple(true, 'yes'), tuple(false, 'no')],
    );
  }

  public function testToShapeWithFloatKeys(): void {
    $map = ThriftMap::forFloat<string>();
    $map->set(1.5, 'one-point-five');
    $map->set(2.5, 'two-point-five');

    $shape = $map->toShape();

    expect($shape)->toHaveSameValuesInAnyOrderAs(
      vec[tuple(1.5, 'one-point-five'), tuple(2.5, 'two-point-five')],
    );
  }

  public function testWithAdaptedStructKey(): void {
    $adapted_key_spec = shape(
      'adapter' => AdapterTestStructToShape::class,
      'type' => TType::STRUCT,
      'class' => AdapterTest\Bar::class,
    );
    $map = new ThriftMap<AdapterTestStructToShape::THackType, string>(
      TType::STRUCT,
      $adapted_key_spec,
    );

    $key1 = shape('str' => '1');
    $key2 = shape('str' => '2');

    $map->set($key1, 'val1');
    $map->set($key2, 'val2');

    expect($map->count())->toEqual(2);
    expect($map->get($key1))->toEqual('val1');
    expect($map->get($key2))->toEqual('val2');
    expect($map->contains($key1))->toBeTrue();

    $shape = $map->toShape();
    expect($shape)->toHaveSameValuesInAnyOrderAs(
      vec[tuple($key1, 'val1'), tuple($key2, 'val2')],
    );

    $restored = ThriftMap::fromShape<
      AdapterTestStructToShape::THackType,
      string,
    >($shape, TType::STRUCT, $adapted_key_spec);
    expect($restored->toShape())->toHaveSameValuesInAnyOrderAs($shape);
  }

  public function testAdaptedKeyValueIdentity(): void {
    $adapted_key_spec = shape(
      'adapter' => AdapterTestStructToShape::class,
      'type' => TType::STRUCT,
      'class' => AdapterTest\Bar::class,
    );
    $map = new ThriftMap<AdapterTestStructToShape::THackType, string>(
      TType::STRUCT,
      $adapted_key_spec,
    );

    $key1 = shape('str' => '42');
    $key2 = shape('str' => '42');

    $map->set($key1, 'first');
    $map->set($key2, 'second');

    expect($map->count())->toEqual(1);
    expect($map->get($key1))->toEqual('second');
  }

  public function testWithAdaptedPrimitiveKey(): void {
    $adapted_key_spec = shape(
      'adapter' => AdapterTestIntToString::class,
      'type' => TType::I32,
    );
    $map = new ThriftMap<AdapterTestIntToString::THackType, string>(
      TType::I32,
      $adapted_key_spec,
    );

    $map->set('42', 'forty-two');
    $map->set('99', 'ninety-nine');

    expect($map->count())->toEqual(2);
    expect($map->get('42'))->toEqual('forty-two');
    expect($map->get('99'))->toEqual('ninety-nine');

    $shape = $map->toShape();
    expect($shape)->toHaveSameValuesInAnyOrderAs(
      vec[tuple('42', 'forty-two'), tuple('99', 'ninety-nine')],
    );

    $restored = ThriftMap::fromShape<AdapterTestIntToString::THackType, string>(
      $shape,
      TType::I32,
      $adapted_key_spec,
    );
    expect($restored->toShape())->toHaveSameValuesInAnyOrderAs($shape);
  }

  public function testToShapeRandomizesIterationOrder(): void {
    $map = new ThriftMap<int, string>(TType::I32, shape('type' => TType::I32));

    foreach (vec[0, 1, 2, 3, 4, 5, 6, 7] as $value) {
      $map->set($value, (string)$value);
    }

    $orders = keyset[];
    for ($i = 0; $i < 20; $i++) {
      $orders[] =
        Str\join(Vec\map($map->toShape(), $entry ==> (string)$entry[0]), ',');
    }

    expect(C\count($orders))->toBeGreaterThan(1);
  }

  public function testRepeatedSetsExposeAllEntriesInAnyOrder(): void {
    $map = $this->createMap();
    $insertion_order = vec[5, 1, 4, 2, 3];
    foreach ($insertion_order as $i) {
      $map->set(SimpleStruct::fromShape(shape('a_i32' => $i)), (string)$i);
    }

    $observed = Vec\map($map->toShape(), $entry ==> $entry[0]->a_i32);
    expect($observed)->toHaveSameValuesInAnyOrderAs(vec[1, 2, 3, 4, 5]);
  }

  public function testGetKeysAndValuesExposeAllEntriesInAnyOrder(): void {
    $map = new ThriftMap<int, string>(TType::I32, shape('type' => TType::I32));

    $map->set(1, 'one');
    $map->set(-1, 'negative-one');
    $map->set(0, 'zero');

    expect($map->getKeys())->toHaveSameValuesInAnyOrderAs(vec[0, -1, 1]);
    expect($map->getValues())
      ->toHaveSameValuesInAnyOrderAs(vec['zero', 'negative-one', 'one']);
  }

  public function testIteratorRandomizesIterationOrder(): void {
    $map = new ThriftMap<int, string>(TType::I32, shape('type' => TType::I32));

    foreach (vec[0, 1, 2, 3, 4, 5, 6, 7] as $value) {
      $map->set($value, (string)$value);
    }

    $orders = keyset[];
    for ($i = 0; $i < 20; $i++) {
      $keys = vec[];
      foreach ($map as $key => $_value) {
        $keys[] = (string)$key;
      }
      $orders[] = Str\join($keys, ',');
    }

    expect(C\count($orders))->toBeGreaterThan(1);
  }

  public function testCopyEntrySetIsMutationIsolated(): void {
    $map = $this->createMap();
    $key = SimpleStruct::fromShape(shape('a_i32' => 1));
    $map->set($key, 'one');

    $copy = $map->copy();
    $map->set($key, 'updated');

    expect($copy->get($key))->toEqual('one');
    expect($map->get($key))->toEqual('updated');
  }

  private static function newSimpleStructWithInt(int $value): SimpleStruct {
    return SimpleStruct::fromShape(shape('a_i32' => $value));
  }

}
