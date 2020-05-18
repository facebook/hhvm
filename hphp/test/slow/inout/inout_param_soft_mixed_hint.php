<?hh

function foo(<<__Soft>> inout mixed $thing) {
  $thing = 5;
}

function main() {
  $a = 2;
  foo(inout $a);
  var_dump($a);
}


<<__EntryPoint>>
function main_inout_param_soft_mixed_hint() {
main();
}
