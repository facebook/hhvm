<?hh // partial

function foo(): int {
  return 1;
}

function concat_it(string $a, int $b): string {
  return sprintf("%s %d", $a, $b);
}

function say_it(int $a): string {
  return sprintf("%d", $a);
}

function piped(): string {
  return foo() |> concat_it((say_it($$) |> say_it($$)), $$);
}
