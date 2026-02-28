<?hh

namespace A {
  class MyAttr implements \HH\FileAttribute {
    public function __construct(public string $attr_value) { }
  }

  class MyClass { }

  <<file: MyAttr(MyClass::class)>>
}

namespace B {
  class MyOtherClass { }
}
