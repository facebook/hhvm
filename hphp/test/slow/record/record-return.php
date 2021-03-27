<?hh

record Foo {
  int f;
}

record Bar {
  int f;
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
