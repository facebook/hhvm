<?hh

function bar(int $a): int {
  return $a + 1;
}

function say_it(int $a): string {
  return sprintf("%d", $a);
}

function echo_it(?string $a): ?string {
  return $a;
}

function piped(): void {
  $z = 5 |?> 3 + $$ |?> say_it($$);

  echo_it($z); // $z is a ?string
}
