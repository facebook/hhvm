<?hh

function f($x) {
  try {
    $r = is_array($x) ? $x[0] : false;
    var_dump($r);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  $x = is_string($x) && isset($x[0]) ? $x[0] : false;
  var_dump($x);
  $x = is_string($x) && isset($x[1]) ? $x[1] : isset($x[0]) ? $x[0] : false;
  var_dump($x);
}

<<__EntryPoint>>
function main_1855() {
f('');
f('foo');
f('f');
f(varray[]);
f(varray[32]);
}
