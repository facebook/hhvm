<?hh

function f($x) {
 return $x;
 }
function foo($a) {
  yield 1;
  foreach ((array)f($a) as $x) {
    var_dump('i:'.$x);
  }
}

<<__EntryPoint>>
function main_2160() {
foreach (foo(varray[1]) as $x) {
  var_dump('o:'.$x);
}
}
