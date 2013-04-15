<?hh

function foo(string $y) : shape('a' => string) {
  // shapes must have compile-time constant key names
  $x = shape($y => 'asd');
}

foo();