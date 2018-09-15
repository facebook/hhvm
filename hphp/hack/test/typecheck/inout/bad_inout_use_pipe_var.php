<?hh // strict

function f(inout int $i): void {
  $i += 1;
}

function test(): int {
  $x = 42;
  $x |> f(inout $$);
  return $x;
}
