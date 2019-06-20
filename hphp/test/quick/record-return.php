<?hh

final record Foo {
  f: int,
}

final record Bar {
  f: int,
}

function foo(int $x): Foo {
  if ($x > 0) {
    return Foo['f' => 1];
  } else {
    return Bar['f' => 1];
  }
}

$a = foo(1);
var_dump($a['f']);
$a = foo(-1);
var_dump($a['f']);
