<?hh

function f(
  TypeStructure<vec<int>> $v,
  TypeStructure<dict<int, string>> $d,
  TypeStructure<keyset<int>> $k,
): void {
  $v["classname"];
  $d["classname"];
  $k["classname"];
}
