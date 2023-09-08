<?hh

function just_nullable_bool(?bool $x): void {
  switch ($x) {
    case null:
      return;
    case true:
      return;
    case false:
      return;
  }
}

function default_true(?bool $x): void {
  switch ($x) {
    case null:
      return;
    case false:
      return;
    default:
      return;
  }
}

function missing_false(?bool $x): void {
  switch ($x) {
    case null:
      return;
    case true:
      return;
  }
}
