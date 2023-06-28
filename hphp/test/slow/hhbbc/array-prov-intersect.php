<?hh

<<__EntryPoint>>
function main() :mixed{
  $x = varray[];
  if (__hhvm_intrinsics\launder_value(true)) {
    $x = darray[100 => 0, 200 => 1, 300 => 2];
  }
  if (__hhvm_intrinsics\launder_value(true) && !isset($x[100])) {
    echo "not set\n";
  } else {
    echo "set\n";
  }
}
