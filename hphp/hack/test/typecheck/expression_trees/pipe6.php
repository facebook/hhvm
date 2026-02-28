<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function bar(ExampleExpression<ExampleInt> $x): ExampleExpression<ExampleString> {
  throw new Exception();
}

function helper_baz(ExampleContext $_):
  Awaitable<ExampleExpression<ExampleFunction<(function(ExampleString): ExampleFloat)>>>
{
  throw new Exception();
}

function helper_foo(ExampleContext $_):
  Awaitable<ExampleExpression<ExampleFunction<(function(ExampleInt): ExampleInt)>>>
{
  throw new Exception();
}

function helper_quux(ExampleContext $_):
  Awaitable<ExampleExpression<ExampleFunction<(function(ExampleFloat): ExampleInt)>>>
{
  throw new Exception();
}

// Ensure that multiple pipes work
function test(): ExampleExpression<ExampleInt> {
  $et = 1
    |> foo($$)
    |> ExampleDsl`${ $$ }`
    |> bar($$)
    |> ExampleDsl`helper_baz(${ $$ })`;

  $et = "hello"
    |> ($$ . 4 . $$)
    |> (int)$$
    |> ExampleDsl`${ foo($$) } + ${ foo($$) }`
    |> ExampleDsl`helper_quux(${ $et }) + helper_foo(${ $$ })`;

  return $et;
}
