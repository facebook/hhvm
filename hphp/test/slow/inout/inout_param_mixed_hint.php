<?hh

function foo(inout mixed $thing) {
  $thing = 5;
}

function main() {
  $a = 2;
  foo(&$a);
  var_dump($a);
}


<<__EntryPoint>>
function main_inout_param_mixed_hint() {
main();
}
