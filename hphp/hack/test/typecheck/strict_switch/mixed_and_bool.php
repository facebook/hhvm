<?hh

function mixed_with_bool_case(mixed $x): void {
  switch ($x) {
    case true:
      return;
    default:
      return;
  }
}

function mixed_only_bool(mixed $x): void {
  switch ($x) {
    case false:
      return;
  }
}
