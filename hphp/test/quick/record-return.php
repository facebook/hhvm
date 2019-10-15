<?hh

record Foo {
  f: int,
}

record Bar {
  f: int,
}

function foo(int $x): Foo {
  if ($x > 0) {
    return Foo['f' => 1];
  } else {
    return Bar['f' => 1];
  }
}
<<__EntryPoint>> function main(): void {
$a = foo(1);
var_dump($a['f']);
$a = foo(-1);
var_dump($a['f']);
}
