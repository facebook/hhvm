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
  $f = mut<>;
  if (true) {
    $f = ro<>;
  } else {
    $f = mut<>;
  }
  $_ = $f(); // ReadonlyViolationException
}
