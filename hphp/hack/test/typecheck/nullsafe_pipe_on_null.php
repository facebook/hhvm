<?hh

function nullsafe_on_null(?int $x): void {
  $x |?> $$ + 42; // OK
  if ($x is null) {
    $x |?> $$ + 42; // Not OK since $x is known to be null
  }
}
