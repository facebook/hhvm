<?hh

function redundant_nullsafe_pipe(?int $x, int $y): void {
  $x |?> $$ + 42; // OK
  $y |> $$ + 42; // OK
  $y |?> $$ + 42; // Not OK since $y is not nullable
}
