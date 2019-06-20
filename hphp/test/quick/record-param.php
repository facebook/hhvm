<?hh

record Foo {
  x: int,
}

record Bar {
  x: int,
}

function foo(Foo $r): Foo {
  $r['x'] = $r['x'] + 1;
  return $r;
}

$f = Foo['x' => 10];
$z = foo($f);
var_dump($z['x']);

$f = Bar['x' => 20];
$z = foo($f);
var_dump($z['x']);
