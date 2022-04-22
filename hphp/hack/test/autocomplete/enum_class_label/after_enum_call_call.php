<?hh

enum class MyEnum: mixed {
  bool a_bool = true;
}

function get_bool(HH\EnumClass\Label<MyEnum, bool> $label): void {}

class Foo {
  public static function bar(): void {}
}

function call_it(): void {
  get_bool(#a_bool);

  Foo::AUTO332
}
