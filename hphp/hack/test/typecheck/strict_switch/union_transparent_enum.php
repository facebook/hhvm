<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints', 'strict_switch')>>

enum IntEnumGAsInt:int as int {
  G = 7;
}

enum StringEnumHAsString:string as string {
  H = "H";
}

<<__StrictSwitch>>
function union_transparent_enum_int((int | IntEnumGAsInt) $x): void {
  switch ($x) {
    case 8:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function union_transparent_enum_string((string | StringEnumHAsString) $x): void {
  switch ($x) {
    case "bar":
      return;
    default:
      return;
  }
}
