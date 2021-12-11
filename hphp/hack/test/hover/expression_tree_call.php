<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

async function bar(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(ExampleString): ExampleInt)>> {
  throw new Exception();
}

function foo(): void {
  ExampleDsl`bar("abc")`;
           // ^ hover-at-caret
}
