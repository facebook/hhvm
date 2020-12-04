<?hh
<<file:__EnableUnstableFeatures('ifc')>>

class A {
  <<__Policied("A")>>
  public function foo() : void {}
}

class B extends A {
  <<__InferFlows>>
  public function foo(): void {}

}
