<?hh

function junk() { return 2; }
function bar() {
  $x = darray['x' => junk()];
  $x['x']++;
  $val = $x['x'];
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_array($x));
  var_dump($x);
}

<<__EntryPoint>>
function main_array_027() {
bar();
}
