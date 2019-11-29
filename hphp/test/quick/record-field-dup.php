<?hh

abstract record A {
  int x;
}

record B extends A {
  int x;
}

<<__EntryPoint>>
function main() {
  $a = B['x' => 10];
}
