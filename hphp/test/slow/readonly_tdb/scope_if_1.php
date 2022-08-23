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
  $a = $a = 2;
  $f = ro<>;
  if (1 > 2) {
    $f = mut<>;
  } else {
    $_ = $f(); // inferred readonly
    $f = ro<>;
  }
  $_ = $f(); // ReadonlyViolationException
}
