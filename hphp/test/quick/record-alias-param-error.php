<?hh

record Foo {
  x: int,
}

record Bar {
  x: int,
}

type Baz = Bar;

function myfunc(Baz $a) : int {
  return $a['x'];
}

<<__EntryPoint>>
function main() {
  $foo = Foo['x' => 10];
  myfunc($foo);
}
