<?hh

function junk() { return 2; }
function bar() {
  $x = darray['y' => junk(), 'x' => darray[]];
  $x['x']['z'] = 0;
  $x['x']['z'] += 1;
  $val = $x['x'];
  $val2 = $x['x']['z'];
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_null($val2));
  var_dump(is_array($val2));
}

<<__EntryPoint>>
function main_array_024() {
bar();
}
