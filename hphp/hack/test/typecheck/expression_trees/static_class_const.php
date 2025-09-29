<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

abstract class MyClass {
  abstract const string MYCONST;

  public function test(): void {
    $fun_call = ExampleDsl`static::MYCONST`;
  }
}
