<?hh

function pid($x) {
  var_dump($x);
  return $x;
}
function f($x) {
  return $x;
}
function ttest() {
  return f(pid('arg1'),pid('arg2'));
}

<<__EntryPoint>>
function main_1843() {
ttest();
}
