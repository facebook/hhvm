<?hh

class A {
  public $a = "hello";
  public $b = vec[1, 2, 3];
}

<<__EntryPoint>>
function main() :mixed{
  apc_store("some-key", new A());
  __hhvm_intrinsics\apc_fetch_no_check("some-key");
  $b = __hhvm_intrinsics\apc_fetch_no_check("some-key");
  $c = clone $b;
  var_dump(__hhvm_intrinsics\launder_value($c->b));
}
