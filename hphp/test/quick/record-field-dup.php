<?hh

abstract record A {
  x: int,
}

final record B extends A {
  x: int,
}

<<__EntryPoint>>
function main() {
  $a = B['x' => 10];
}
