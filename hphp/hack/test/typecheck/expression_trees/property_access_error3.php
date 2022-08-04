<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(MyState $x) ==> {
    foo(1)->my_prop = 1;
  }`;
}

async function foo(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(ExampleInt): ExampleInt)>>> {
  throw new Exception();
}

abstract class MyState {
  public ExampleInt $my_prop;
}
