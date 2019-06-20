<?hh

abstract record A {
  x: int,
}

<<__EntryPoint>>
function main() {
  $a = A['x' => 10];
}
