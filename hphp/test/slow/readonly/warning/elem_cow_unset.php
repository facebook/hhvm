<?hh

<<file:__EnableUnstableFeatures('readonly')>>

class Foo1 {
  public readonly vec<vec<int>> $b = vec[vec[0],vec[1],vec[2]];

  public readonly vec<Vector<vec<int>>> $bad_b =
    vec[Vector {vec[0], vec[1], vec[2]}];
}

class Foo {
  public vec<Foo1> $c;
}

class Bar {
  public Foo $myFoo;
}

<<__EntryPoint>>
function test(): void {
  $b = new Bar();
  $b->myFoo = new Foo();
  $b->myFoo->c = vec[new Foo1()];
  unset($b->myFoo->c[0]->b[0][0]);
  unset($b->myFoo->c[0]->bad_b[0][0][0]);
}
