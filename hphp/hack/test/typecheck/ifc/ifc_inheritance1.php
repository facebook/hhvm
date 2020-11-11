<?hh

class A {
  <<__Policied("A")>>
  public function foo() : void {}
}

class B extends A {
  <<__Policied("B")>>
  public function foo(): void {}

}
