<?hh

function foo() {
  $x = new stdClass();
  $x->foo = "heh";
  return $x;
}

function bar() {
  var_dump(foo());
}




<<__EntryPoint>>
function main_minstr_002() {
bar();
}
