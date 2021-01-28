<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// A regular server side async/awaitable function
async function my_gk_example():
  Awaitable<Spliceable<Code, Code::TAst, ExampleInt>>
{
  throw new Exception();
}

// A helper function that calls the async function and splices in the result
async function my_helper(
  ExampleContext $_,
): Awaitable<ExprTree<Code, Code::TAst, (function(): ExampleInt)>> {
  $f = await my_gk_example();
  return Code`() ==> ${ $f }`;
}

// Use the helper function in an expression tree
function test(): void {
  Code`my_helper()`;
}
