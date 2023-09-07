<?hh

function just_null(null $x): void {
  switch ($x) {
    case null:
      return;
  }
}

function non_literal(null $x): void {
  $y = null;

  switch ($x) {
    case $y:
      return;
  }
}
