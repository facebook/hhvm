<?hh

class C {
  <<Foo("hi " . "there")>>
  public function attrStr() {}

  <<Foo(0x42, 123)>>
  public function attrInt();

  <<Foo(SomeClass#Label1, #Label2)>>
  public function attrECLabel() {}

  <<Foo(SomeClass::class)>>
  public function attrClsName() {}

  <<Foo(21 * 2)>>
  public function attrCompute() {}

  <<Foo('bar'.Baz::class)>>
  public function attrConcat() {}

  <<Foo(vec[1, 2, 3])>>
  public function attrVec() {}

  <<Foo(dict['a' => 1, 'b' => 2])>>
  public function attrDict() {}

  <<Foo(keyset[1, 2, 3])>>
  public function attrKeyset() {}

  <<Foo(shape('a' => 1, 'b' => 2))>>
  public function attrShape() {}
}
