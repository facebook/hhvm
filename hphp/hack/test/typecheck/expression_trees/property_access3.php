<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`(MyState $x) ==> {
    foo(1)->my_prop = 1;
  }`;
}

async function foo(ExampleContext $_): Awaitable<ExprTree<Code, Code::TAst, (function(ExampleInt): MyState)>> {
  throw new Exception();
}

abstract class MyState {
  public ExampleInt $my_prop;
}
