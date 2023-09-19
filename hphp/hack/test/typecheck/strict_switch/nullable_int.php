<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function just_nullable_int_literal_default(?int $x): void {
  switch ($x) {
    case null:
      return;
    case 42:
      return;
    default:
        return;
  }
}

<<__StrictSwitch>>
function just_nullable_int_default(?int $x): void {
  switch ($x) {
    case null:
      return;
    default:
        return;
  }
}

<<__StrictSwitch>>
function nullable_int_not_null(?int $x): void {
  if ($x is null) {
    return;
  }

  switch ($x) {
    case 42:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function nullable_int_not_int(?int $x): void {
  if ($x is int) {
    return;
  }

  switch ($x) {
    case null:
      return;
  }
}
