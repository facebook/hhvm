<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class MyParent {
  public static async function bar(
    ExampleContext $_,
  ): Awaitable<ExprTree<Code, Code::TAst, (function(ExampleString): ExampleInt)>> {
    throw new Exception();
  }
}

class MyClass extends MyParent {
  public function test(): void {
    $fun_call = Code`parent::bar("baz")`;
  }
}
