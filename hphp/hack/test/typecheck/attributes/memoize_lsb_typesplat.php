<?hh
<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

class Foo<T as (mixed...)> {
  <<__MemoizeLSB>>
  public static function bar(...T $args): int {
    return 42;
  }
}
