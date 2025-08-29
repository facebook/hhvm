<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function bar(ExampleExpression<ExampleInt> $x): ExampleExpression<ExampleString> {
  throw new Exception();
}

function helper_baz(ExampleContext $_):
  Awaitable<ExampleExpression<(function(ExampleString): ExampleFloat)>>
{
  throw new Exception();
}

function test(): ExampleExpression<ExampleInt> {
  // Lambdas don't capture $$, so we should error here
  $et = 1 |> ExampleDsl`${ (() ==> { return foo($$); })() }`;
  return $et;
}
