<?hh

class Foo {
  public function bar<T1>(T1 $x): T1 {
    return $x;
  }
}

function f(): int {
  $foo = new Foo();
  $res = $foo->bar<int>(1);
  return 0;
}
