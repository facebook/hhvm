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
  $g = ro<>;
  try {
    $f = ro<>;
    $g = ro<>;
    $_ = $f(); // inferred readonly
    $_ = $g(); // inferred readonly
  } catch (Exception $e) {
    $f = mut<>;
    $g = mut<>;
  } finally {
    $f = ro<>;
  }

  $foo_ro = $f(); // inferred readonly
  $foo_mut = $g(); // no inferred readonly, ReadonlyViolationException
}
