<?hh

function f($x) :mixed{
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
function g($x) :mixed{
  if (!is_array($x)) return;
  var_dump($x[0]);
}
function h($x) :mixed{
  if (!is_array($x) && !is_string($x)) {
    var_dump('1');
  }
 else {
    var_dump($x[0]);
  }
}
function i($x) :mixed{
  return !is_array($x) ? $x[0] : $x[0];
}

<<__EntryPoint>>
function main_1859() :mixed{
f(vec[0, 1, 2]);
g(vec[0, 1, 2]);
h(vec[0, 1, 2]);
h('foobar');
h(new stdClass());
var_dump(i(vec[0, 1, 2]));
}
