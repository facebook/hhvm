<?hh

interface Foo {
  public static function bar(int $x): void;
}

function qux<reify T as Foo>(): void {
  $x = T::bar<>;
  $x('hello');
}
