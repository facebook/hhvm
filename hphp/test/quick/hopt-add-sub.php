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
$vals = varray[
  varray[0, 0],
  varray[1, 1],
  varray[-3, 5],
  varray[5, -3],
  varray[20, -50],
  varray[20, 0],
];

foreach ($vals as list($a, $b)) {
  var_dump(foo1($a, $b));
  var_dump(foo2($a, $b));
  var_dump(foo3($a, $b));
  var_dump(foo4($a, $b));
}
}
