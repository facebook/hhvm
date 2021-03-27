<?hh

abstract record A {
  int x;
}

record B extends A {
  int y;
}

class Foo {
  public A $f;
}

function foo(A $a): A {
  $a['x'] = 42;
  return $a;
}

<<__EntryPoint>>
function main() {
  $b = B['x' => 10, 'y' => 20];
  $a = foo($b);
  var_dump($a['x']);
  var_dump($a['y']);

  $foo = new Foo();
  $foo->f = $b;
  var_dump($foo->f['x']);
}
