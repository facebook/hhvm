<?hh

function double_null(null $x): void {
  switch ($x) {
    case null:
      return;
    case null:
      return;
  }
}
