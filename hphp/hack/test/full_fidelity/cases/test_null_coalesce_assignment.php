<?hh

function foo(int $y): int {
  return $y;
}

function f(?int $x): int {
  $x ??= 0;
  $y = 3;
  return $x ??= $y |> foo();
}
