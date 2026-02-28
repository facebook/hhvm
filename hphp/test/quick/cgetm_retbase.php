<?hh
function f() :mixed{ $a = vec[]; $a[] = 1; return $a; }
function nonHphpArray( ) :mixed{
  apc_add('foo', vec[1, 2, 3]);
  return __hhvm_intrinsics\apc_fetch_no_check('foo');
}
function test1() :mixed{
  return f()[0];
}
<<__EntryPoint>> function main(): void {
  var_dump(test1());
  var_dump(nonHphpArray()[0]);
}
