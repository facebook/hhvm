<?hh

final class Test {
  public static function foo<reify T>() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__EntryPoint>>
function main() :mixed{
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
