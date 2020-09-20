<?hh

abstract record A {
  int x;
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
