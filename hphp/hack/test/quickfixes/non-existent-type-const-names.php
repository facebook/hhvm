<?hh

class MyClass {
  const type TFooBar = int;
  const type TFoo = int;
  public function foo(): MyClass::TFooBaz {
    throw new Exception();
  }
}
