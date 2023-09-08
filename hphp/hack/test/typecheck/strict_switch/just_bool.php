<?hh

function just_bool(bool $x): void {
  switch ($x) {
    case true:
      return;
    case false:
      return;
  }
}

function non_literal(bool $x): void {
  $y = true;

  switch ($x) {
    case $y:
      return;
    case false:
      return;
  }
}

function redundant_default(bool $x): void {
  switch ($x) {
    case true:
      return;
    case false:
      return;
    default:
      return;
  }
}

function redundant_false(bool $x): void {
  switch ($x) {
    case false:
      return;
    case true:
      return;
    case false:
        return;
  }
}

function missing_true(bool $x): void {
  switch ($x) {
    case false:
      return;
  }
}
