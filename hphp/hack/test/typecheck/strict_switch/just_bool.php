<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function just_bool(bool $x): void {
  switch ($x) {
    case true:
      return;
    case false:
      return;
  }
}

<<__StrictSwitch>>
function non_literal(bool $x): void {
  $y = true;

  switch ($x) {
    case $y:
      return;
    case false:
      return;
  }
}

<<__StrictSwitch>>
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

<<__StrictSwitch>>
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

<<__StrictSwitch>>
function missing_true(bool $x): void {
  switch ($x) {
    case false:
      return;
  }
}
