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
  $f = ro<>;
  for ($i = 0; $i < 10; $i++) {
    $f = ro<>;
    $g = mut<>;
  }
  $_ = $g();   // no inferred readonly
  $foo = $f(); // inferred readonly
  $foo->x = 2; // fatal
}
