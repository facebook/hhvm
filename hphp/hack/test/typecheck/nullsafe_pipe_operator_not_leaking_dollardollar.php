<?hh

function foo(): int {
  return 1;
}

function bar(int $a): int {
  return $a + 1;
}

function piped(): ?int {
  $i = foo() |?> bar($$);
  if ($i is null) {
    return null;
  }
  return bar($$) + $i;
}
