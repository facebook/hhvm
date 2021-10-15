<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

function bar(ExprTree<Code, Code::TAst, ExampleInt> $x): ExprTree<Code, Code::TAst, ExampleString> {
  throw new Exception();
}

function helper_baz(ExampleContext $_):
  Awaitable<ExprTree<Code, Code::TAst, (function(ExampleString): ExampleFloat)>>
{
  throw new Exception();
}

function helper_foo(ExampleContext $_):
  Awaitable<ExprTree<Code, Code::TAst, (function(ExampleInt): ExampleInt)>>
{
  throw new Exception();
}

function helper_quux(ExampleContext $_):
  Awaitable<ExprTree<Code, Code::TAst, (function(ExampleFloat): ExampleInt)>>
{
  throw new Exception();
}

// Ensure that multiple pipes work
function test(): ExprTree<Code, Code::TAst, ExampleInt> {
  $et = 1
    |> foo($$)
    |> Code`${ $$ }`
    |> bar($$)
    |> Code`helper_baz(${ $$ })`;

  $et = "hello"
    |> ($$ . 4 . $$)
    |> (int)$$
    |> Code`${ foo($$) } + ${ foo($$) }`
    |> Code`helper_quux(${ $et }) + helper_foo(${ $$ })`;

  return $et;
}
