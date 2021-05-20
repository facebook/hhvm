<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public static async function bar(ExampleContext $_):
    Awaitable<ExprTree<Code, Code::TAst, (function(): noreturn)>>
  {
    throw new Exception();
  }
}

function baz(ExampleContext $_):
  Awaitable<ExprTree<Code, Code::TAst, (function(): noreturn)>>
{
  throw new Exception();
}

function test(): void {
  Code`Foo::bar()`;
  Code`baz()`;
}

function test2(): ExprTree<Code, Code::TAst, noreturn> {
  return Code`baz()`;
}
