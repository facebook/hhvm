<?hh

function foo1($a, $b) :mixed{
  return ($a - 4) + ($b - 9);
}

function foo2($a, $b) :mixed{
  return ($a - 3) + 7;
}

function foo3($a, $b) :mixed{
  return ($a - 3) - 7;
}

function foo4($a, $b) :mixed{
  return ($a - 4) + ($b + 9);
}
<<__EntryPoint>> function main(): void {
$vals = vec[
  vec[0, 0],
  vec[1, 1],
  vec[-3, 5],
  vec[5, -3],
  vec[20, -50],
  vec[20, 0],
];

foreach ($vals as list($a, $b)) {
  var_dump(foo1($a, $b));
  var_dump(foo2($a, $b));
  var_dump(foo3($a, $b));
  var_dump(foo4($a, $b));
}
}
