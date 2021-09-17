<?hh
//// def.php
// definition is in separate file
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public function __construct(
    public int $prop,
  ){}
}

class Bar {
  public static readonly vec<Foo> $foo_vec_static = vec[];
  public static readonly Vector<Foo> $foo_vector_static = Vector {};

  public function __construct(
    public readonly Foo $foo,
    public readonly vec<Foo> $foo_vec,
    public readonly Vector<Foo> $foo_vector
  ) {}
}

//// test.php
function test(Bar $b) : void {
  $b->foo->prop = 4;
  $b->foo_vec[] = new Foo(1); // ok
  $b->foo_vector[] = new Foo(1); // not ok
  Bar::$foo_vec_static[]  = new Foo(1); // ok
  Bar::$foo_vector_static[] = new Foo(1); // not ok
  Bar::$foo_vec_static[0]->prop = 4; // error
}
