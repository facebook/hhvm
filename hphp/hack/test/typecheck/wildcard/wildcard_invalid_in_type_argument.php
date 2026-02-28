<?hh

class C<reify T> { }

function identity<T>(T $x): T {
  return $x;
}

function identity_explicit<<<__Explicit>> T>(T $x): T {
  return $x;
}

function test1():void {
  $x = identity_explicit<_>(3);
  $y = identity_explicit<vec<_>>(vec[3]);
  $z = identity<C<_>>(new C<int>());
}

class D {
  public function m1(vec<_> $x):void { }
  public function m2():vec<_> { }
}
