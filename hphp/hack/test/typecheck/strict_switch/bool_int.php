<?hh


<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>
<<file: __EnableUnstableFeatures('strict_switch')>>

<<__StrictSwitch>>
function union_bool_int((bool | int) $x): void {
  switch ($x) {
    case 0:
      return;
    case true:
      return;
    case false:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function union_bool_int_no_int_default((bool | int) $x): void {
  switch ($x) {
    case true:
      return;
    case false:
      return;
  }
}
