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
  for ($i = 0; $i < 10; $i++) {
    if (1 < 2) {
      break;
    }
    $f = mut<>;
  }
  $_ = $f(); // no inferred readonly, ReadonlyViolationException
}
