<?hh // partial

function foo(): int {
  return 1;
}

function bar(int $a): int {
  return $a + 1;
}

function say_it(int $a): string {
  return sprintf("%d", $a);
}

function piped(): string {
  return foo() |> bar($$) + $$ |> say_it($$);
}
