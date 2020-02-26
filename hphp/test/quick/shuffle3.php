<?hh
function f() { $a = varray[]; $a[] = 1; return $a; }
function nonHphpArray( ) {
  apc_add('foo', varray[1, 2, 3]);
  return __hhvm_intrinsics\apc_fetch_no_check('foo');
}
function test1() {
  return f()[0];
}
<<__EntryPoint>> function main(): void {
  var_dump(test1());
}
