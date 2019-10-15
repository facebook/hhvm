<?hh

abstract record A {
  x: int,
}

record B extends A {
  x: int,
}

<<__EntryPoint>>
function main() {
  $a = B['x' => 10];
}
