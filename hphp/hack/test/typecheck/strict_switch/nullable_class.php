<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
class C {}

<<__StrictSwitch>>
function nullable_class(?C $x): void {
  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function nullable_class_not_null(?C $x): void {
  if ($x is null) {
    return;
  }

  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function case_null_return(?C $x): void {
  switch ($x) {
    case null:
      return;
    default:
      break;
  }

  switch ($x) {
    // Because of control flow, this is a redundant case
    // However, type inference misses this
    // Hence still a safe overapproximation
    case null:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function nullable_class_not_class(?C $x): void {
  if ($x is C) {
    return;
  }

  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}
