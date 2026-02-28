<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function bar(ExampleExpression<ExampleInt> $x): ExampleExpression<ExampleString> {
  throw new Exception();
}

function test(): ExampleExpression<ExampleString> {
  $et = 1
    |> $$ . "Hello" . $$
    |> (int)$$
    |> ExampleDsl`${ foo($$) |> bar($$) }`;

  return $et;
}
