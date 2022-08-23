<?hh

class Foo {
  public int $x = 0;
}
function ro(): readonly Foo {
  return new Foo();
}

function mut(): Foo {
  return new Foo();
}

<<__EntryPoint>>
function main(): void {
  $f = ro<>;
  if (1 > 2) {
    $f = mut<>;
  }
  $foo = $f();
  $foo->setX(); // fatal
}
