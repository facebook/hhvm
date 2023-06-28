<?hh

function f1($x) :mixed{
  var_dump($x[0]);
  var_dump($x[0]);
  var_dump($x[0]);
}
function f2($x) :mixed{
  $i = 3;
  while ($x[0] && $i--) {
    var_dump($x[0]);
  }
}
function f3($x, $y) :mixed{
  do {
    var_dump($x[0]);
  }
 while ($x[0] && $y);
  var_dump($x[0]);
}
function f4($x) :mixed{
  foreach ($x[0] as $k => $v) {
    var_dump($x[0]);
    var_dump($k, $v);
  }
}
function f5($x) :mixed{
  switch ($x[0]) {
  case (bool)0:
    var_dump($x[0]);
  }
}
function f6($x, $y, $z) :mixed{
  if ($y) {
 var_dump($y);
 }
  else if ($x[0]) {
    var_dump($x[0]);
    var_dump($x);
  }
}

<<__EntryPoint>>
function main_543() :mixed{
f1(varray[0, 0]);
f2(varray[10]);
f3(varray[10], false);
f4(varray[varray[1, 2, 3]]);
f5(varray[false, false]);
f6(varray[true], false, false);
}
