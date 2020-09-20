<?hh

final class Test {
  public static function foo<reify T>(int $a) {
    echo "Test::foo($a)\n";
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__Memoize>>
function memo_fptr($func, int $x) {
  $func($x);
}

<<__EntryPoint>>
function main() {
  memo_fptr(Test::foo<int>, 1);
  memo_fptr(__hhvm_intrinsics\launder_value(Test::foo<int>), 2);

  // Skip execution and side effects of these
  memo_fptr(Test::foo<int>, 1);
  memo_fptr(__hhvm_intrinsics\launder_value(Test::foo<int>), 2);
}
