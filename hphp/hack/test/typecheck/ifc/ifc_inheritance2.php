<?hh
<<file:__EnableUnstableFeatures('ifc')>>

class A {
  <<__Policied("A")>>
  public function foo() : void {}
}

class B extends A {
  // Default public policy
  public function foo(): void {}

}
