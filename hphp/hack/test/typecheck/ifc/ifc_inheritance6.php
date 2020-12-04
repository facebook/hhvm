<?hh
<<file:__EnableUnstableFeatures('ifc')>>

class C {}
trait TA {
  <<__Policied("A")>>
  public function foo(<<__External>> C $x): void {
  }
}

class N {
  <<__Policied("A")>>
  public function foo(C $x): void {

  }
}

class M extends N {
  use TA; // TA overrides foo, but $x <: External $x so we are good
}
