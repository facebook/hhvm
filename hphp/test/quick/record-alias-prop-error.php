<?hh

final record Foo {
  x: int,
}

final record Bar {
  x: int,
}

type Baz = Bar;

class C {
  public Baz $f;
}

<<__EntryPoint>>
function main() {
  $foo = Foo['x' => 10];
  $o = new C;
  $o->f = $foo;
}
