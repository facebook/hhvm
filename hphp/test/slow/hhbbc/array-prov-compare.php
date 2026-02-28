<?hh

<<__EntryPoint>>
function main() :mixed{
  $a = __hhvm_intrinsics\launder_value(null);
  $b = __hhvm_intrinsics\launder_value(null);
  $c = __hhvm_intrinsics\launder_value(null);

  if (tuple($a, $b, $c) === tuple(null, null, null)) {
    echo "same\n";
  } else {
    echo "not same\n";
  }
}
