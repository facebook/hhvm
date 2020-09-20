<?hh

function lol() { return STDIN; }
function foo() {
  $x = lol();
  unset($x[0]['id']);
  return $x;
}


<<__EntryPoint>>
function main_elemu() {
error_reporting(-1);

var_dump(foo());
}
