<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class MyParent {
  const string MYCONST = "Hello";
}

class MyClass extends MyParent {
  public function test(): void {
    $fun_call = ExampleDsl`parent::MYCONST`;
  }
}
