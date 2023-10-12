<?hh // strict

function map(
  Map<int, string> $m,
  Map<int, string> $mm,
)[]: void {
  $m[1] = "42"; // ERROR
  $mm[0] = "0"; // ERROR
}

function vector(
  Vector<string> $v,
  Vector<string> $mv,
)[]: void {
  $v[1] = "42"; // ERROR
  $mv[0] = "0"; // ERROR
}
