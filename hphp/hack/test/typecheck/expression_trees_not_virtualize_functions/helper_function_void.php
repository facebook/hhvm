<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public static async function bar(ExampleContext $_):
    Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(): void)>>
  {
    throw new Exception();
  }
}

function baz(ExampleContext $_):
  Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(): void)>>
{
  throw new Exception();
}

function test(): void {
  ExampleDsl`Foo::bar()`;
  ExampleDsl`baz()`;
}

function test2(): ExprTree<ExampleDsl, ExampleDsl::TAst, void> {
  return ExampleDsl`baz()`;
}
