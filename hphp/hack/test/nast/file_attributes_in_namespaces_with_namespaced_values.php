<?hh

namespace A {
  class MyStringFileAttribute implements \HH\FileAttribute {
    public function __construct(public string $attr_value) { }
  }

  class MyClass {
    const string MY_CLASS_CONST = 'my class const value'
  }

  <<file: MyStringFileAttribute(MyClass::MY_CLASS_CONST)>>
}

namespace B {
  class MyOtherClass { }
}
