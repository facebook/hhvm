<?hh

final record Foo {
  x:int,
}

$f = Foo['x' => 1];
$a = 10;
$f['x'] =& $a;
$a = 100;
var_dump($f['x']);
