<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExprTree<Code, Code::TAst, ExampleInt> {
  throw new Exception();
}

function bar(ExprTree<Code, Code::TAst, ExampleInt> $x): ExprTree<Code, Code::TAst, ExampleString> {
  throw new Exception();
}

function test(): ExprTree<Code, Code::TAst, ExampleString> {
  $et = 1
    |> $$ . "Hello" . $$
    |> (int)$$
    |> Code`${ foo($$) |> bar($$) }`;

  return $et;
}
