<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function nullable_bool_not_null(?bool $x): void {
  if ($x is null) {
    return;
  }

  switch ($x) {
    case true:
      return;
    case false:
      return;
  }
}

<<__StrictSwitch>>
function case_null_return(?bool $x): void {
  switch ($x) {
    case null:
      return;
    default:
      break;
  }

  switch ($x) {
    // Because of control flow, null is redundant case
    // However, type inference misses this
    // Hence is expected to error
    case true:
      return;
    case false:
      return;
  }
}
