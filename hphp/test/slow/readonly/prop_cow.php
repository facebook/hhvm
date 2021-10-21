<?hh

class Foo {

  public readonly vec<vec<vec<int>>> $c = vec[vec[vec[0],vec[1],vec[2]]];
  public readonly vec<Vector<vec<int>>> $bad_c = vec[Vector {vec[0],vec[1],vec[2]}];
}

class Bar {
  public Foo $myFoo;
}

<<__EntryPoint>>
function test(): void {
  $b = new Bar();
  $b->myFoo = new Foo();
  $b->myFoo->c[0][0][0] = 10;
  $b->myFoo->bad_c[0][0][0] = 20;
}
