<?hh

class Foo {
  public A $x;
}

record A {
  int x;
}

record B {
  int x;
}
<<__EntryPoint>> function main(): void {
$a = A['x' => 1];
$b = B['x' => 2];

$foo = new Foo();
$foo->x = $a;
var_dump($foo->x['x']);

$foo->x = $b;
var_dump($foo->x['x']);
}
