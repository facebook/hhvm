<?hh

function f($x) {
  if (is_array($x) && isset($x[0])) {
 var_dump($x[0]);
 }
  else if (is_string($x) && $x && $x[0]) {
 var_dump($x[0]);
 }
  else if (is_integer($x)) {
 var_dump($x);
 }
}

<<__EntryPoint>>
function main_1854() {
f(varray[32]);
f('foobar');
f(32);
}
