<?hh

abstract record A {
  x: int,
}

record B extends A {
}

record C extends A {
}

function foo(B $b) {
  var_dump($b['x']);
}

<<__EntryPoint>>
function main() {
  $c = C['x' => 1];
  foo($c);
}
