<?hh

function id($x) {
  return $x;
}
function f1($x) {
  $z = id($x[0]);
  foreach ($x[0] as $a) {
    $z[] = varray[id($z), count($x[0])];
  }
}

function f2($x) {
  var_dump($x[0]);
  $y = 'foo' . $x[0] . 'bar';
}

function f3($x) {
  $x = is_string($x[0]) ? $x[0] : get_class($x[0]);
  return $x;
}

<<__EntryPoint>>
function main_539() {
f1(varray[varray[0, 1, 2, 3]]);
f2('foobar');
var_dump(f3('abc'));
var_dump(f3(varray[new stdClass]));
}
