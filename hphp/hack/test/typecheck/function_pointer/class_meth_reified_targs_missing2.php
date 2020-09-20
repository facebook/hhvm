<?hh

final class Foo {
  public static function bar<reify T, reify T2>(T $_): void {}
}

function test(): void {
  $x = Foo::bar<int>;
}
