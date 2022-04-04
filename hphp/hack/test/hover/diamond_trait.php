<?hh


trait MyTrait1 {
  public function testFun(): void {}
}
trait MyTrait2 {
  use MyTrait1;
}

<<__EnableMethodTraitDiamond>>
// ^ hover-at-caret
class MyClass {
  use MyTrait1;
  use MyTrait2;
}
