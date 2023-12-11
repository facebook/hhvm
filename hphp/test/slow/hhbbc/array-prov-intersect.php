<?hh

<<__EntryPoint>>
function main() :mixed{
  $x = vec[];
  if (__hhvm_intrinsics\launder_value(true)) {
    $x = dict[100 => 0, 200 => 1, 300 => 2];
  }
  if (__hhvm_intrinsics\launder_value(true) && !isset($x[100])) {
    echo "not set\n";
  } else {
    echo "set\n";
  }
}
