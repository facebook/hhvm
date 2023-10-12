<?hh // strict

function id<T>(T $x): T {
  return $x;
}

function f(): string {
  $x = new stdClass();
  $x->foo = 1;
  $y = id($x);
  return $y->foo;
}
