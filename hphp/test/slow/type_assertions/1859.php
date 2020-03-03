<?hh

function f($x) {
  if (!is_array($x)) {
    var_dump($x[0]);
  }
 else if (isset($x[0])) {
    var_dump($x[0]);
  }
  if (!!!is_array($x)) {
    var_dump($x[0]);
  }
 else if (isset($x[0])) {
    var_dump($x[0]);
  }
}
function g($x) {
  if (!is_array($x)) return;
  var_dump($x[0]);
}
function h($x) {
  if (!is_array($x) && !is_string($x)) {
    var_dump('1');
  }
 else {
    var_dump($x[0]);
  }
}
function i($x) {
  return !is_array($x) ? $x[0] : $x[0];
}

<<__EntryPoint>>
function main_1859() {
f(varray[0, 1, 2]);
g(varray[0, 1, 2]);
h(varray[0, 1, 2]);
h('foobar');
h(new stdClass());
var_dump(i(varray[0, 1, 2]));
}
