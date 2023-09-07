<?hh

function default_null(null $x): void {
  switch ($x) {
    default:
      return;
  }
}
