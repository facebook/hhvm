<?hh

<<__EntryPoint>>
function main() :mixed{
  $x = darray[];
  if (__hhvm_intrinsics\launder_value(true)) {
    $x = darray['a' => 123, 'b' => 456];
  }
  if (__hhvm_intrinsics\launder_value(true) && $x) {
    var_dump($x);
  } else {
    echo "empty\n";
  }
}
