<?hh

<<__EntryPoint>>
function main() :mixed{
  $x = __hhvm_intrinsics\launder_value(shape('max' => null, 'min' => 123));
  if ($x is shape('max' => null, 'min' => num)) {
    echo "Match\n";
  } else {
    echo "No Match\n";
  }

  $x = __hhvm_intrinsics\launder_value(tuple(null, 123));
  if ($x is (null, num)) {
    echo "Match\n";
  } else {
    echo "No Match\n";
  }
}
