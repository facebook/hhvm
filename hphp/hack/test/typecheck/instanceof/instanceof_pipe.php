<?hh // strict

class C {}

function f(mixed $x): void {
  $x |> $$ instanceof C ? expect_C($$) : null;
}

function expect_C(C $c): void {}
