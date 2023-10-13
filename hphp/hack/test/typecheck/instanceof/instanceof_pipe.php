<?hh

class C {}

function f(mixed $x): void {
  $x |> $$ is C ? expect_C($$) : null;
}

function expect_C(C $c): void {}
