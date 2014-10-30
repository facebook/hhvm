<?hh

type Foo = shape();

function any(): mixed {
  // UNSAFE
}

function f() {
  $x = any();
  if ($x instanceof string) {
    return 'dead code';
  }
  if ($x instanceof Foo) {
    return 'dead code';
  }
}
