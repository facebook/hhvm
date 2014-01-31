<?hh

function main() {
  $val = 'value';

  $v = Vector {'v1', 'v2'};
  $m = Map {'k1' => 'v1', 'k2' => 'v2'};
  $s = Set {'v1', 'v2'};
  $a = array('v1', 'v2');
  var_dump(array_fill_keys($v, $val));
  var_dump(array_fill_keys($m, $val));
  var_dump(array_fill_keys($s, $val));
  var_dump(array_fill_keys($a, $val));

  $v = Vector {};
  $m = Map {};
  $s = Set {};
  var_dump(array_fill_keys($v, $val));
  var_dump(array_fill_keys($m, $val));
  var_dump(array_fill_keys($s, $val));
}
main();
