<?hh

final class Baz {
  public static int $bar = 42;
}

enum class Foo: mixed {
  int BAR = Baz::$bar; // illegal, missing read_props
}
