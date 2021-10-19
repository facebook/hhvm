<?hh

class Foo {
  public readonly vec<int> $c = vec[vec[0], vec[1], vec[2]];
  public readonly Vector<int> $bad_c = Vector {vec[0], vec[1], vec[2]};
}

class Bar {
  public Foo $foo;
  public readonly Foo $ro_foo;
}

<<__EntryPoint>>
function test(): void {
  $a = new Bar();
  $a->foo = new Foo();
  $a->ro_foo = new Foo();
  $a->foo->c[0][0] = 10; // ok
  try {
    $a->ro_foo->c[0][0] = 10; // error
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  $a->foo->bad_c[0][0] = 10; // error
}
