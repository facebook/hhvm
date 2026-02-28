<?hh

function foo(bool $coinflip): ?string {
  return $coinflip ? "foo" : null;
}

function bar(string $prefix): string {
  return $prefix."bar";
}

function normal_pipe(bool $coinflip): void {
  // Not ok since bar expects string not ?string
  foo($coinflip) |> bar($$);
}

function nullsafe_pipe(bool $coinflip): void {
  // OK because of nullsafe pipe operator
  $result = foo($coinflip) |?> bar($$);
  // Not ok since bar expects string not ?string
  bar($result);
}
