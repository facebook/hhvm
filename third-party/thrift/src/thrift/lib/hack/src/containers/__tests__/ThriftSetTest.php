<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
final class ThriftSetTest extends WWWTest {

  private function getStructElemSpec(): ThriftStructTypes::TGenericSpec {
    return shape(
      'type' => TType::STRUCT,
      'class' => SimpleStruct::class,
    );
  }

  private function createSet(): ThriftSet<SimpleStruct> {
    return new ThriftSet(TType::STRUCT, $this->getStructElemSpec());
  }

  public function testAddAndContains(): void {
    $set = $this->createSet();
    $elem =
      SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));
    $set->add($elem);

    expect($set->contains($elem))->toBeTrue();
    expect($set->count())->toEqual(1);
  }

  public function testValueBasedDedup(): void {
    $set = $this->createSet();
    $elem1 =
      SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));
    $elem2 =
      SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));

    $set->add($elem1);
    $set->add($elem2);

    // Same struct values = dedup
    expect($set->count())->toEqual(1);
  }

  public function testDifferentElementsAreDifferent(): void {
    $set = $this->createSet();
    $elem1 = SimpleStruct::fromShape(shape('a_i32' => 1));
    $elem2 = SimpleStruct::fromShape(shape('a_i32' => 2));

    $set->add($elem1);
    $set->add($elem2);

    expect($set->count())->toEqual(2);
    expect($set->contains($elem1))->toBeTrue();
    expect($set->contains($elem2))->toBeTrue();
  }

  public function testRemove(): void {
    $set = $this->createSet();
    $elem = SimpleStruct::fromShape(shape('a_i32' => 42));
    $set->add($elem);

    expect($set->contains($elem))->toBeTrue();
    $set->remove($elem);
    expect($set->contains($elem))->toBeFalse();
    expect($set->count())->toEqual(0);
  }

  public function testIsEmpty(): void {
    $set = $this->createSet();
    expect($set->isEmpty())->toBeTrue();

    $elem = SimpleStruct::fromShape(shape('a_i32' => 42));
    $set->add($elem);
    expect($set->isEmpty())->toBeFalse();
  }

  public function testIterator(): void {
    $set = $this->createSet();
    $elem1 = SimpleStruct::fromShape(shape('a_i32' => 1));
    $elem2 = SimpleStruct::fromShape(shape('a_i32' => 2));

    $set->add($elem1);
    $set->add($elem2);

    $items = vec[];
    foreach ($set->getIterator() as $elem) {
      $items[] = $elem->a_i32;
    }

    expect(C\count($items))->toEqual(2);
    expect($items)->toContain(1);
    expect($items)->toContain(2);
  }

  public function testForeach(): void {
    $set = $this->createSet();
    $elem1 = SimpleStruct::fromShape(shape('a_i32' => 1));
    $elem2 = SimpleStruct::fromShape(shape('a_i32' => 2));

    $set->add($elem1);
    $set->add($elem2);

    $items = vec[];
    foreach ($set as $elem) {
      $items[] = $elem->a_i32;
    }

    expect(C\count($items))->toEqual(2);
    expect($items)->toContain(1);
    expect($items)->toContain(2);
  }

  public function testForeachCopiesElements(): void {
    $set = $this->createSet();
    $elem =
      SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));
    $set->add($elem);

    foreach ($set as $e) {
      $e->a_i32 = 999;
    }

    foreach ($set as $e) {
      expect($e->a_i32)->toEqual(42);
    }
  }

  public function testMutableCollectionElementsAreCopied(): void {
    $nested_list_spec = shape(
      'type' => TType::LST,
      'etype' => TType::I32,
      'elem' => shape('type' => TType::I32),
      'format' => 'collection',
    );
    $list_elem_spec = shape(
      'type' => TType::LST,
      'etype' => TType::LST,
      'elem' => $nested_list_spec,
      'format' => 'collection',
    );
    $set = new ThriftSet<Vector<Vector<int>>>(TType::LST, $list_elem_spec);
    $nested_element = Vector {1, 2};
    $element = Vector {$nested_element};

    $set->add($element);
    $nested_element[] = 3;

    expect($set->contains(Vector {Vector {1, 2}}))->toBeTrue();
    expect($set->contains($element))->toBeFalse();
  }

  public function testMapElementUsesSerializedContents(): void {
    $map_elem_spec = shape(
      'type' => TType::MAP,
      'ktype' => TType::I32,
      'vtype' => TType::STRING,
      'key' => shape('type' => TType::I32),
      'val' => shape('type' => TType::STRING),
      'format' => 'collection',
    );
    $set = new ThriftSet<Map<int, string>>(TType::MAP, $map_elem_spec);
    $element = Map {2 => 'two', 1 => 'one'};

    $set->add($element);
    $element[3] = 'three';

    $equivalent_element = Map {1 => 'one', 2 => 'two'};
    expect($set->contains($equivalent_element))->toBeTrue();
    expect($set->contains($element))->toBeFalse();
  }

  public function testSetElementUsesSerializedContents(): void {
    $set_elem_spec = shape(
      'type' => TType::SET,
      'etype' => TType::I32,
      'elem' => shape('type' => TType::I32),
      'format' => 'collection',
    );
    $set = new ThriftSet<Set<int>>(TType::SET, $set_elem_spec);
    $element = Set {2, 1};

    $set->add($element);
    $element[] = 3;

    $equivalent_element = Set {1, 2};
    expect($set->contains($equivalent_element))->toBeTrue();
    expect($set->contains($element))->toBeFalse();
  }

  public function testObjectKeyMapElementUsesSerializedContents(): void {
    $object_key_map_spec =
      HH\FIXME\UNSAFE_CAST<mixed, ThriftStructTypes::TGenericSpec>(
        shape(
          'type' => TType::MAP,
          'ktype' => TType::STRUCT,
          'vtype' => TType::STRING,
          'key' => $this->getStructElemSpec(),
          'val' => shape('type' => TType::STRING),
          'format' => 'object_key',
        ),
        'Nested object-key map spec matches generated shape.',
      );
    $set = new ThriftSet<ThriftMap<SimpleStruct, string>>(
      TType::MAP,
      $object_key_map_spec,
    );
    $element = new ThriftMap<SimpleStruct, string>(
      TType::STRUCT,
      $this->getStructElemSpec(),
    );
    $element->set(SimpleStruct::fromShape(shape('a_i32' => 2)), 'two');
    $element->set(SimpleStruct::fromShape(shape('a_i32' => 1)), 'one');

    $set->add($element);
    $element->set(SimpleStruct::fromShape(shape('a_i32' => 3)), 'three');

    $equivalent_element = new ThriftMap<SimpleStruct, string>(
      TType::STRUCT,
      $this->getStructElemSpec(),
    );
    $equivalent_element->set(
      SimpleStruct::fromShape(shape('a_i32' => 1)),
      'one',
    );
    $equivalent_element->set(
      SimpleStruct::fromShape(shape('a_i32' => 2)),
      'two',
    );
    expect($set->contains($equivalent_element))->toBeTrue();
    expect($set->contains($element))->toBeFalse();
  }

  public function testStructElementsAreDeepCopied(): void {
    $set = new ThriftSet<ComplexStruct>(
      TType::STRUCT,
      shape('type' => TType::STRUCT, 'class' => ComplexStruct::class),
    );
    $element = ComplexStruct::fromShape(
      shape('simple_struct_list' => Vector {self::newSimpleStructWithInt(1)}),
    );

    $set->add($element);
    $element->simple_struct_list[] = self::newSimpleStructWithInt(2);

    $original_element = ComplexStruct::fromShape(
      shape('simple_struct_list' => Vector {self::newSimpleStructWithInt(1)}),
    );
    expect($set->contains($original_element))->toBeTrue();
    expect($set->contains($element))->toBeFalse();
  }

  public function testToVec(): void {
    $set = $this->createSet();
    $elem1 = SimpleStruct::fromShape(shape('a_i32' => 1));
    $elem2 = SimpleStruct::fromShape(shape('a_i32' => 2));

    $set->add($elem1);
    $set->add($elem2);

    $vec = $set->toVec();
    expect(Vec\map($vec, $elem ==> $elem->a_i32))
      ->toHaveSameValuesInAnyOrderAs(vec[1, 2]);
  }

  public function testWithBoolElements(): void {
    $bool_spec = shape('type' => TType::BOOL);
    $set = new ThriftSet<bool>(TType::BOOL, $bool_spec);

    $set->add(true);
    $set->add(false);
    $set->add(true); // duplicate

    expect($set->count())->toEqual(2);
    expect($set->contains(true))->toBeTrue();
    expect($set->contains(false))->toBeTrue();
  }

  public function testWithFloatElements(): void {
    $float_spec = shape('type' => TType::DOUBLE);
    $set = new ThriftSet<float>(TType::DOUBLE, $float_spec);

    $set->add(1.5);
    $set->add(2.5);
    $set->add(1.5); // duplicate

    expect($set->count())->toEqual(2);
    expect($set->contains(1.5))->toBeTrue();
    expect($set->contains(2.5))->toBeTrue();
  }

  public function testWithUnionElements(): void {
    $union_spec = shape(
      'type' => TType::STRUCT,
      'class' => SimpleUnion::class,
    );
    $set = new ThriftSet<SimpleUnion>(TType::STRUCT, $union_spec);

    $u1 = SimpleUnion::fromShape(shape('a_bool' => true));
    $u2 = SimpleUnion::fromShape(shape('a_byte' => 5));

    $set->add($u1);
    $set->add($u2);

    expect($set->count())->toEqual(2);
    expect($set->contains($u1))->toBeTrue();
    expect($set->contains($u2))->toBeTrue();
  }

  public function testCountable(): void {
    $set = $this->createSet();
    expect(PHP\count($set))->toEqual(0);

    $elem = SimpleStruct::fromShape(shape('a_i32' => 42));
    $set->add($elem);
    expect(PHP\count($set))->toEqual(1);
  }

  public function testForStruct(): void {
    $set = ThriftSet::forStruct<SimpleStruct>();
    $elem =
      SimpleStruct::fromShape(shape('a_i32' => 42, 'a_string' => 'hello'));
    $set->add($elem);

    expect($set->contains($elem))->toBeTrue();
    expect($set->count())->toEqual(1);
  }

  public function testForBool(): void {
    $set = ThriftSet::forBool();

    $set->add(true);
    $set->add(false);
    $set->add(true); // duplicate

    expect($set->count())->toEqual(2);
    expect($set->contains(true))->toBeTrue();
    expect($set->contains(false))->toBeTrue();
  }

  public function testForFloat(): void {
    $set = ThriftSet::forFloat();

    $set->add(1.5);
    $set->add(2.5);
    $set->add(1.5); // duplicate

    expect($set->count())->toEqual(2);
    expect($set->contains(1.5))->toBeTrue();
    expect($set->contains(2.5))->toBeTrue();
  }

  public function testWithAdaptedStructElements(): void {
    $adapted_spec = shape(
      'adapter' => AdapterTestStructToShape::class,
      'type' => TType::STRUCT,
      'class' => AdapterTest\Bar::class,
    );
    $set = new ThriftSet<AdapterTestStructToShape::THackType>(
      TType::STRUCT,
      $adapted_spec,
    );

    $elem1 = shape('str' => '1');
    $elem2 = shape('str' => '2');

    $set->add($elem1);
    $set->add($elem2);
    $set->add($elem1); // duplicate

    expect($set->count())->toEqual(2);
    expect($set->contains($elem1))->toBeTrue();
    expect($set->contains($elem2))->toBeTrue();

    $vec = $set->toVec();
    expect($vec)->toHaveSameValuesInAnyOrderAs(vec[$elem1, $elem2]);

    $restored = ThriftSet::fromVec<AdapterTestStructToShape::THackType>(
      $vec,
      TType::STRUCT,
      $adapted_spec,
    );
    expect($restored->toVec())->toHaveSameValuesInAnyOrderAs($vec);
  }

  public function testWithAdaptedPrimitiveElements(): void {
    $adapted_spec = shape(
      'adapter' => AdapterTestIntToString::class,
      'type' => TType::I32,
    );
    $set = new ThriftSet<AdapterTestIntToString::THackType>(
      TType::I32,
      $adapted_spec,
    );

    $set->add('42');
    $set->add('99');
    $set->add('42'); // duplicate

    expect($set->count())->toEqual(2);
    expect($set->contains('42'))->toBeTrue();
    expect($set->contains('99'))->toBeTrue();

    $vec = $set->toVec();
    expect($vec)->toHaveSameValuesInAnyOrderAs(vec['42', '99']);

    $restored = ThriftSet::fromVec<AdapterTestIntToString::THackType>(
      $vec,
      TType::I32,
      $adapted_spec,
    );
    expect($restored->toVec())->toHaveSameValuesInAnyOrderAs($vec);
  }

  public function testFromVec(): void {
    $elem1 = SimpleStruct::fromShape(shape('a_i32' => 10));
    $elem2 = SimpleStruct::fromShape(shape('a_i32' => 20));
    $elements = vec[$elem1, $elem2];

    $set = ThriftSet::fromVec<SimpleStruct>(
      $elements,
      TType::STRUCT,
      $this->getStructElemSpec(),
    );

    expect($set->count())->toEqual(2);
    expect($set->contains($elem1))->toBeTrue();
    expect($set->contains($elem2))->toBeTrue();
  }

  public function testFromVecRejectsDuplicateElements(): void {
    $elem1 = SimpleStruct::fromShape(shape('a_i32' => 42));
    $elem2 = SimpleStruct::fromShape(shape('a_i32' => 42));

    expect(
      () ==> ThriftSet::fromVec<SimpleStruct>(
        vec[$elem1, $elem2],
        TType::STRUCT,
        $this->getStructElemSpec(),
      ),
    )->toThrow(InvariantViolationException::class, 'Duplicate element');
  }

  public function testToVecFromVecRoundTrip(): void {
    $set = $this->createSet();
    $elem1 =
      SimpleStruct::fromShape(shape('a_i32' => 1, 'a_string' => 'hello'));
    $elem2 =
      SimpleStruct::fromShape(shape('a_i32' => 2, 'a_string' => 'world'));

    $set->add($elem1);
    $set->add($elem2);

    $restored = ThriftSet::fromVec<SimpleStruct>(
      $set->toVec(),
      TType::STRUCT,
      $this->getStructElemSpec(),
    );

    expect($restored->count())->toEqual(2);
    expect($restored->contains($elem1))->toBeTrue();
    expect($restored->contains($elem2))->toBeTrue();
  }

  public function testFromVecWithBoolElements(): void {
    $bool_spec = shape('type' => TType::BOOL);
    $set = ThriftSet::fromVec<bool>(vec[true, false], TType::BOOL, $bool_spec);

    expect($set->count())->toEqual(2);
    expect($set->contains(true))->toBeTrue();
    expect($set->contains(false))->toBeTrue();
  }

  public function testFromVecRejectsDuplicateBoolElements(): void {
    expect(
      () ==> ThriftSet::fromVec<bool>(
        vec[true, false, true],
        TType::BOOL,
        shape('type' => TType::BOOL),
      ),
    )->toThrow(InvariantViolationException::class, 'Duplicate element');
  }

  public function testBoolIterationExposesAllEntriesInAnyOrder(): void {
    $set = ThriftSet::forBool();
    $set->add(false)->add(true);

    expect($set->toVec())->toHaveSameValuesInAnyOrderAs(vec[true, false]);
  }

  public function testRepeatedAddsExposeAllEntriesInAnyOrder(): void {
    $set = $this->createSet();
    $insertion_order = vec[5, 1, 4, 2, 3];
    foreach ($insertion_order as $i) {
      $set->add(SimpleStruct::fromShape(shape('a_i32' => $i)));
    }

    $observed = Vec\map($set->toVec(), $e ==> $e->a_i32);
    expect($observed)->toHaveSameValuesInAnyOrderAs(vec[1, 2, 3, 4, 5]);
  }

  public function testCopyIsMutationIsolated(): void {
    $set = $this->createSet();
    $element = SimpleStruct::fromShape(shape('a_i32' => 1));
    $set->add($element);

    $copy = $set->copy();
    $set->remove($element);

    expect($copy->contains($element))->toBeTrue();
    expect($set->contains($element))->toBeFalse();
  }

  private static function newSimpleStructWithInt(int $value): SimpleStruct {
    return SimpleStruct::fromShape(shape('a_i32' => $value));
  }
}
