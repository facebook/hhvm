class MyClass {
  const type TFooBar = int;
  public function foo(this::TFooBaz $_) {
    throw new Exception();
  }
}