<?hh // strict

<<__Pure>>
function map(
  Map<int, string> $m,
  <<__Mutable>> Map<int, string> $mm,
)[]: void {
  $m[1] = "42"; // ERROR
  $mm[0] = "0"; // ERROR
}

<<__Pure>>
function vector(
  Vector<string> $v,
  <<__Mutable>> Vector<string> $mv,
)[]: void {
  $v[1] = "42"; // ERROR
  $mv[0] = "0"; // ERROR
}
