<?hh

class A {
  static public function func1() { return 1; }
}

<<__EntryPoint>>
function main() {
  $k = 'keya';
  apc_store($k, HH\class_meth(A::class, 'func1'));
  var_dump(apc_fetch($k));
}
