<?hh

class A {
  static public function func1() { return 1; }
}

<<__EntryPoint>>
function main() {
  $k = 'keya';
  apc_store($k, A::func1<>);
  var_dump(__hhvm_intrinsics\apc_fetch_no_check($k));
}
