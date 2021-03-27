<?hh

abstract record Foo {
  int x;
}

type Bar = Foo;

record Baz extends Foo {
}

function foo(Bar $a) : Bar{
  $a['x'] = 42;
  return $a;
}

<<__EntryPoint>>
function main() {
  $a = Baz['x' => 10];
  $a = foo($a);
  var_dump($a['x']);
}
