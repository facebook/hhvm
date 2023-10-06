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
}
