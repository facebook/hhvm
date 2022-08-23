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
  do {
    if (1 > 2) {
      break;
    } else {
      $f = ro<>;
    }
  } while (false);

    // note: this code doesn't type-check
    $foo = $f(); // ReadonlyViolationException
}
