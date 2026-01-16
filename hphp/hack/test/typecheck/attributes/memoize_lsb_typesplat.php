<?hh


class Foo<T as (mixed...)> {
  <<__MemoizeLSB>>
  public static function bar(...T $args): int {
    return 42;
  }
}
