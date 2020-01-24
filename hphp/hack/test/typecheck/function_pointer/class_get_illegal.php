<?hh

final class Foo {
  public static function bar(int $x): void {}
}

function qux(): void {
  $x = Foo::{'bar' . ''}<>;
  $x(4);
}

<<__EntryPoint>>
function main(): void {
  qux();
}
