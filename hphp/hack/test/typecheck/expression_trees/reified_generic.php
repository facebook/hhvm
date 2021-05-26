<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public static async function bar(ExampleContext $_):
    Awaitable<ExprTree<Code, Code::TAst, (function(): void)>>
  {
    throw new Exception();
  }
}

function takes_reified<reify T as Foo>(): void {
  Code`T::bar()`;
}
