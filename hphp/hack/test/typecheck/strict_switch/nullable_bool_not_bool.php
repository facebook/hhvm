<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function nullable_bool_not_bool(?bool $x): void {
  if ($x is bool) {
    return;
  }

  switch ($x) {
    case null:
      return;
  }
}

<<__StrictSwitch>>
function case_true_false_return(?bool $x): void {
  switch ($x) {
    case true:
      return;
    case false:
      return;
    default:
      break;
  }

  switch ($x) {
    // Because of control flow, true/false are redundant cases
    // However, type inference misses this
    // Hence is expected to error
    case null:
      return;
  }
}
