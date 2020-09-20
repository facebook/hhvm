<?hh

final class Foo {
  public function bar(string $x): void {
    echo $x;
  }
}

function baz(): void {
  $x = new Foo();
  $y = $x->bar<>;
  $y("Hello World!\n");
}

<<__EntryPoint>>
function main(): void {
  baz();
}
