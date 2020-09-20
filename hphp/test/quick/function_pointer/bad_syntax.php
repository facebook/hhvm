<?hh

final class Foo {
  public static function bar(int $x): void {}
}

function adze(): void {
  $z = 'bar';
  $x = Foo::$z<>;
  $x(4);
}

<<__EntryPoint>>
function main(): void {
  adze();
}
