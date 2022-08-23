<?hh

class Foo {
  public int $x = 0;
}

function ro(): readonly Foo {
  return new Foo();
}

function ro2(): readonly Foo {
  return new Foo();
}

<<__EntryPoint>>
function main(): void {
  $a = $a = 2;
  $f = ro<>;
  if (1 < 2) {
    $f = ro2<>;
  }
  $foo = $f(); // inferred readonly
  $foo->x = 3; // fatal
}
