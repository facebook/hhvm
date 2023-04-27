<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

async function bar(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(): void)>> {
  throw new Exception();
}
async function baz(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(ExampleInt): void)>> {
  throw new Exception();
}

function foo(): void {
  $loop = ExampleDsl`() ==> { while(true) { bar(); } }`;
  $for = ExampleDsl`() ==> { for($x = 0; true; $x = $x + 1) { baz($x); } }`;
}
