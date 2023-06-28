<?hh

function foo(inout mixed $thing) :mixed{
  $thing = 5;
}

function main() :mixed{
  $a = 2;
  foo(inout $a);
  var_dump($a);
}


<<__EntryPoint>>
function main_inout_param_mixed_hint() :mixed{
main();
}
