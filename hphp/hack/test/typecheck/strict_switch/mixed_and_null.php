<?hh

function mixed_with_null_case(mixed $x): void {
  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}

function mixed_only_null(mixed $x): void {
  switch ($x) {
    case null:
      return;
  }
}
