<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class B { }
class C extends B {
  public function foo():void { }
}
class D extends B {
}
class Inv<Tinv> {
  public function __construct(private Tinv $item) { }
}

class Contra<-Tc> {
  public function __construct(private Tc $item) { }
}

function fst<T>(T $x, Contra<T> $y): T {
  return $x;
}

function bar(C $c, Contra<D> $d): Inv<B> {
  // Here, let $x:v and C<:v and Contra<D><:Contra<v> so v <: D
  $x = fst($c, $d);
  // Here, we find v:=C but we must also check C<:D and fail!
  $x->foo();
  // Here, we must instantiate Tinv=B
  return new Inv($x);
}
