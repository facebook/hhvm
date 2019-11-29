<?hh

abstract record A {
  int x;
}

<<__EntryPoint>>
function main() {
  $a = A['x' => 10];
}
