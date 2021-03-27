<?hh

record Foo {
  int x;
}

record Bar {
  int x;
}

type Baz = Bar;

function myfunc(int $a) : Baz {
  return Foo['x' => $a];
}

<<__EntryPoint>>
function main() {
  myfunc(10);
}
