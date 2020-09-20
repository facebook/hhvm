<?hh // partial

function bar(int $a) {
  return $a + 1;
}

function say_it(int $a): string {
  return sprintf("%d", $a);
}

function echo_it(string $a): string {
  return $a;
}

function piped(): void {
  $z = 5 |> $y = 3 + $$ |> say_it($$);
}
