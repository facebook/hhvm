<?hh

function f(array $a, $e) {
  $a[$e] = darray[$e => 30];
  $x = new stdClass();
  $x = $a[$e];
  var_dump($a, $x);
}
function g(string $x) {
  var_dump($x[0]);
  var_dump($x[1]);
  var_dump($x[0]);
}

<<__EntryPoint>>
function main_535() {
f(varray[], 0);
g('bar');
g('');
g('b');
}
