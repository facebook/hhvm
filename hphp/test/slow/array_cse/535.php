<?hh

function f(darray $a, $e) :mixed{
  $a[$e] = dict[$e => 30];
  $x = new stdClass();
  $x = $a[$e];
  var_dump($a, $x);
}
function g(string $x) :mixed{
  var_dump($x[0]);
  var_dump($x[1]);
  var_dump($x[0]);
}

<<__EntryPoint>>
function main_535() :mixed{
f(dict[], 0);
g('bar');
g('');
g('b');
}
