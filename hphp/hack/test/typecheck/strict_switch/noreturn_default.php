<?hh

function noreturn_default(mixed $x): void {
  if ($x is int) {
    if ($x is string) {
      switch ($x) {
        default:
          return;
      }
    }
  }
  return;
}
