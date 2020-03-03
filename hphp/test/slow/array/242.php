<?hh

function test($q, $a, $b, $c) {
  $x = darray[0 => $a, 'foo'=> $a];
  if ($x) {
    var_dump(isset($x[0][1]), isset($x['foo'][1]));
    var_dump(isset($x[$b][1]), isset($x[$c][1]));
    $xx = $x[0];
    var_dump(end(inout $xx));
  }
}

<<__EntryPoint>>
function main_242() {
test(5, varray[0,1], 0, 'foo');
}
