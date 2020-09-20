<?hh

record Foo {
  int x;
}

record Bar {
  int x;
}

function foo(Foo $r): Foo {
  $r['x'] = $r['x'] + 1;
  return $r;
}
<<__EntryPoint>> function main(): void {
$f = Foo['x' => 10];
$z = foo($f);
var_dump($z['x']);

$f = Bar['x' => 20];
$z = foo($f);
var_dump($z['x']);
}
