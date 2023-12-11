<?hh

function darray_recursive($x) :mixed{
  if (!HH\is_any_array($x)) {
    return $x;
  }
  $result = dict[];
  foreach ($x as $k => $v) {
    $result[$k] = darray_recursive($v);
  }
  return $result;
}

<<__EntryPoint>>
function main(): void {
$a = vec[vec['A'], vec['B'], vec['C'], vec['D']];
$b = vec[$a, $a, $a, $a];
$c = vec[$b, $b, $b, $b];
$d = vec[$c, $c, $c, $c];
$e = vec[$d, $d, $d, $d];
var_dump(json_encode($e, 0, 3));
var_dump(json_encode($e, 0, 4));
var_dump(json_encode($e, 0, 5));
var_dump(json_encode($e, 0, 6));
var_dump(json_encode($e, 0, 7));
var_dump(json_encode($e));
$E = json_decode(json_encode($e));
var_dump($E == darray_recursive($e));
}
