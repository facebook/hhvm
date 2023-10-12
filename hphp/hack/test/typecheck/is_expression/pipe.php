<?hh // strict

function f(mixed $x): void {
  $x |> $$ is int ? expect_int($$) : null;
}

function expect_int(int $x): void {}
