<?hh

final class Foo {
  public static function bar(string $x): void {
    echo $x;
  }
}

function test(): void {
  $x = Foo::bar<>;
  $x("Hello World!\n");
}

<<__EntryPoint>>
function main(): void {
  test();
}
