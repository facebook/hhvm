<?hh


<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

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

function union_bool_int_no_int_default((bool | int) $x): void {
  switch ($x) {
    case true:
      return;
    case false:
      return;
  }
}
