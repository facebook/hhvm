<?hh

class MyBox<T> {
  public function __construct(public T $value)[] {}
}

enum class MyEnum: mixed {
  MyBox<bool> a_bool = new MyBox(true);
  MyBox<int> not_bool = new MyBox(123);
  MyBox<bool> other_bool = new MyBox(false);
}

class Foo {
  const type TParams = MyEnum;

  private function getBool(
    HH\EnumClass\Label<this::TParams, MyBox<bool>> $label,
  ): void {}

  protected function genRun(): void {
    $this->getBool(#AUTO332
  }
}
