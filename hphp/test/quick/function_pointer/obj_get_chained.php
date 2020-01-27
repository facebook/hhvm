<?hh

final class Foo {
  public function bar(): Foo {
    return $this;
  }

  public function baz(string $x): void {
    echo $x;
  }
}

function qux(): void {
  $x = new Foo();
  $y = $x->bar()->bar()->bar()->baz<>;
  $y("Hello World!\n");
}

<<__EntryPoint>>
function main(): void {
  qux();
}
