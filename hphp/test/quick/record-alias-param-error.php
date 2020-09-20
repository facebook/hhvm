<?hh

record Foo {
  int x;
}

record Bar {
  int x;
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
