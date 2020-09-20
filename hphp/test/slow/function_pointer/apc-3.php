<?hh

final class Test {
  public static function foo<reify T>() {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__EntryPoint>>
function main(): void {
  Test::foo<int>();

  $foo = Test::foo<int>;
  apc_store('foo', $foo);

  $retrieve = __hhvm_intrinsics\apc_fetch_no_check('foo');
  $retrieve();
}
