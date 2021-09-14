<?hh

<<file:__EnableUnstableFeatures('readonly')>>
class Foo {

  public readonly vec<vec<vec<int>>> $c = vec[vec[vec[0],vec[1],vec[2]]];
}

class Bar {
  public Foo $myFoo;
}

<<__EntryPoint>>
function test(): void {
  $b = new Bar();
  $b->myFoo = new Foo();
  $x = $b->myFoo->c;
}
