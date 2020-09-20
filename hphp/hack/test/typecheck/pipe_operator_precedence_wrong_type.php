<?hh // partial

function foo(): int {
  return 1;
}

function bar(int $a): int {
  return $a + 1;
}

function say_it(string $a): string {
  return $a;
}

function piped(): string {
  return foo() |> bar($$) + $$ |> say_it($$);
}
