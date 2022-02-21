<?hh

class MyClass {
  const type TFooBar = int;
  const type TFoo = int;
  public function foo(this::TFooBaz $_): MyClass::TFooBaz {
    throw new Exception();
  }
}