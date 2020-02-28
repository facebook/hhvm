<?hh

function block() {
}
function f($x) {
  if (is_int($x) || is_array($x)) {
    var_dump($x[0]);
  }
}
function g($x) {
  $x = (array) $x;
  block();
  var_dump($x[0]);
}

<<__EntryPoint>>
function main_1858() {
f(varray[10]);
g(varray[10]);
}
