<?hh

final class Test {
  public static function foo<reify T>() {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__EntryPoint>>
function main() {
 $c = __hhvm_intrinsics\apc_fetch_no_check('c');

 if ($c === false) {
   echo "Not in APC\n";
   $c = Test::foo<int>;
   $c();
   apc_store('c', $c);
 } else {
   echo "In APC\n";
 }

 $c();
}
