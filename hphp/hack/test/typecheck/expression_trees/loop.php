<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

async function bar(
  ExampleContext $_,
): Awaitable<ExprTree<Code, Code::TAst, (function(): void)>> {
  throw new Exception();
}
async function baz(
  ExampleContext $_,
): Awaitable<ExprTree<Code, Code::TAst, (function(ExampleInt): void)>> {
  throw new Exception();
}

function foo(): void {
  $loop = Code`() ==> { while(true) { bar(); } }`;
  $for = Code`() ==> { for($x = 0; true; $x = $x + 1) { baz($x); } }`;
}
