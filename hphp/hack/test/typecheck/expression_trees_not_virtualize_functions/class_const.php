<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyTestClass {
  const MYCONST = 'Hello';

  public function test(): void {
    ExampleDsl`MyTestClass::MYCONST`;
  }
}
