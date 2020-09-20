<?hh

class C {}
function foo() { return mt_rand() ? varray[new C] : varray[new C, new C]; }
function val() {
  $x = '0';
  apc_store('asd', $x);
  return __hhvm_intrinsics\apc_fetch_no_check('asd');
}
function bar() {
  $x = foo();
  return $x[\HH\array_key_cast(val())];
}
function main() {
  var_dump(bar());
}

<<__EntryPoint>>
function main_array_046() {
main();
}
