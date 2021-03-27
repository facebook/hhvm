<?hh

record Foo {
  vec x;
}

class A {
  public Foo $f;
}

record Bar {
  A y;
}

record Baz {
  int z;
}
<<__EntryPoint>> function main(): void {
$a = vec[Foo['x' => vec[10]], Foo['x' => vec[42]]];
$a[1]['x'] = vec[50];
var_dump($a[1]['x'][0]);

$foo = Foo['x' => vec[1, 2]];
$foo['x'][0] = 42;
var_dump($foo['x'][0]);

$o = new A;
$o->f = Foo['x' => vec[1]];
$o->f['x'] = vec[42];
var_dump($o->f['x'][0]);

$bar = Bar['y' => new A];
$bar['y']->f = Foo['x' => vec[42]];
$bar['y']->f['x'] = vec[10];
var_dump($bar['y']->f['x']);

$baz = Baz['z' => 10];
$baz['z']++;
$baz['z'] -=2;
var_dump($baz['z']);
}
