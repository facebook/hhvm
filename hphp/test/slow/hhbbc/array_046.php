<?hh

class C {}
function foo() :mixed{ return mt_rand() ? vec[new C] : vec[new C, new C]; }
function val() :mixed{
  $x = '0';
  apc_store('asd', $x);
  return __hhvm_intrinsics\apc_fetch_no_check('asd');
}
function bar() :mixed{
  $x = foo();
  return $x[\HH\array_key_cast(val())];
}
function main() :mixed{
  var_dump(bar());
}

<<__EntryPoint>>
function main_array_046() :mixed{
main();
}
