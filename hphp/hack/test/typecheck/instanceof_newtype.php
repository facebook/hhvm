//// file1.php
<?hh
class Foo {
  public $field;
}

newtype MyFoo as Foo = Foo;

function make(Foo $x): ?MyFoo {
  return $x->field > 0 ? $x : null;
}

//// file2.php
<?hh // strict

function test(mixed $x, MyFoo $y): MyFoo {
  invariant($x instanceof $y, '');
  hh_show($x);
  return $x;
}
