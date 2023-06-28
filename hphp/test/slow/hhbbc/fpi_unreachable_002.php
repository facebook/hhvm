<?hh

function func1($x) :mixed{ return __hhvm_intrinsics\launder_value($x); }
function func2($x) :mixed{ throw new Exception('foo'); }
function main() :mixed{
  var_dump(func1(true) ? func1(123) : func2(456));
}


<<__EntryPoint>>
function main_fpi_unreachable_002() :mixed{
main();
}
