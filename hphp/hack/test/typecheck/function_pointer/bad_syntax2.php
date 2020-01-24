<?hh

final class Foo {
  public function bar(int $x): void {}
}

function qux(): void {
  $z = new Foo();
  $y = 'bar';
  $x = $z->$y<>;
  $x(4);
}

<<__EntryPoint>>
function main(): void {
  qux();
}
