<?hh

function missing_null(null $x): void {
  switch ($x) {
    case 42:
      return;
  }
}
