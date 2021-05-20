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

function test<T>(Spliceable<Code, Code::TAst, (function(T): T)> $x): void {
  Code`${ $x }(Foo::bar())`;
  Code`${ $x }(baz())`;
}
