<?hh

final record Foo {
  x: int,
}

<<__EntryPoint>>
function main() {
  $a = Foo@['x' => 10];
}
