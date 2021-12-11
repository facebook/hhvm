<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

function bar(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleString> {
  throw new Exception();
}

function helper_baz(ExampleContext $_):
  Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(ExampleString): ExampleFloat)>>
{
  throw new Exception();
}

function helper_foo(ExampleContext $_):
  Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(ExampleInt): ExampleInt)>>
{
  throw new Exception();
}

function helper_quux(ExampleContext $_):
  Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(ExampleFloat): ExampleInt)>>
{
  throw new Exception();
}

// Ensure that multiple pipes work
function test(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
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
