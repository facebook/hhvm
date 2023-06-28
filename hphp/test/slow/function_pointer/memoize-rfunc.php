<?hh

function foo<reify T>(int $a) :mixed{
  echo "foo($a)\n";
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

<<__Memoize>>
function memo_fptr($func, int $x) :mixed{
  $func($x);
}

<<__EntryPoint>>
function main() :mixed{
  memo_fptr(foo<int>, 1);
  memo_fptr(__hhvm_intrinsics\launder_value(foo<int>), 2);

  // Skip execution and side effects of these
  memo_fptr(foo<int>, 1);
  memo_fptr(__hhvm_intrinsics\launder_value(foo<int>), 2);
}
