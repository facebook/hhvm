<?hh

function foo($a, $b, $c, $d) {
  var_dump($a, $b, $c, $d);
}
function f($a) {
  foo($a[0], $a[0], $a[0], $a[0]++);
  foo($a[0], $a[0], $a[0], $a[0]);
}
<<__EntryPoint>>
function main_540() {
f(varray[0]);
}
