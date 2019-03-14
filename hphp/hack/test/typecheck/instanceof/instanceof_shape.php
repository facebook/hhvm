<?hh // partial

type Foo = shape();

function any(): mixed {
  // UNSAFE
}

function f() {
  $x = any();
  if ($x instanceof Foo) {
    return 'dead code';
  }
}
