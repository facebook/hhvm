<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// A regular server side async/awaitable function
async function my_gk_example():
  Awaitable<Spliceable<ExampleDsl, ExampleDsl::TAst, ExampleInt>>
{
  throw new Exception();
}

// A helper function that calls the async function and splices in the result
async function my_helper(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(): ExampleInt)>>> {
  $f = await my_gk_example();
  return ExampleDsl`() ==> ${ $f }`;
}

// Use the helper function in an expression tree
function test(): void {
  ExampleDsl`my_helper()`;
}
