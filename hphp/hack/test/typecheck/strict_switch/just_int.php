<?hh

function just_int(int $x): void {
  switch ($x) {
    case 0:
      return;
    default:
      return;
  }
}

function non_literal(int $x): void {
  $y = 0;

  switch ($x) {
    case $y:
      return;
    default:
      return;
  }
}

function missing_default(int $x): void {
  switch ($x) {
    case 0:
      return;
  }
}

function redundant_literal(int $x): void {
  switch ($x) {
    case 0:
      return;
    case 1:
      return;
    case 0:
      return;
    case 1:
      return;
    default:
      return;
  }
}
