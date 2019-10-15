<?hh

record Foo {
  x: int,
}

record Bar {
  x: int,
}

type Baz = Bar;

function myfunc(int $a) : Baz {
  return Foo['x' => $a];
}

<<__EntryPoint>>
function main() {
  myfunc(10);
}
