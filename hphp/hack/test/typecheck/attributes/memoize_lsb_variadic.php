<?hh

class Foo {
  <<__MemoizeLSB>>
  public static function bar(mixed ...$args): int {
    return 42;
  }
}
