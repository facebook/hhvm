<?hh

enum class E: int {
  int A = 40;
  int B = 42;
}

<<__Memoize>>
function foo($x): mixed {
  return $x;
}

<<__EntryPoint>>
function main(): void {
  var_dump(json_encode(E#A));
  apc_store('label', E#A);
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('label'));
  var_dump(foo(E#A));
}
