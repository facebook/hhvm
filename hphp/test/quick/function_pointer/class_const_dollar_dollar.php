<?hh

final class Foo {
  public static function bar(string $x): void {
    echo $x;
  }
}

function baz(): void {
  $x = Foo::class |> $$::bar<>;
  $x('Hello world!\n');
}

<<__EntryPoint>>
function main(): void {
  baz();
}
