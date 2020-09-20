<?hh

function f($x) {
  while (is_array($x) && isset($x[0])) $x = $x[0];
  var_dump($x);
}
function g($x) {
  for (;
 is_array($x) && isset($x[0]);
 $x = $x[0]);
  var_dump($x);
}
function h($x) {
  if (!is_array($x) || !isset($x[0])) return;
  do {
    $x = $x[0];
  }
 while (is_array($x) && isset($x[0]));
  var_dump($x);
}

<<__EntryPoint>>
function main_1857() {
f(varray[varray[varray[varray['hello']]]]);
g(varray[varray[varray[varray['hello']]]]);
h(varray[varray[varray[varray['hello']]]]);
}
