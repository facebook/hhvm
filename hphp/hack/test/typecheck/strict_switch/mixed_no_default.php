<?hh

function mixed_no_default(mixed $x): void {
  switch ($x) {
    case vec[]:
      return;
  }
}
