<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main() :mixed{
  $x = shape('a' => 123);
  if ($x is Shape) echo "Shape\n";
  if ($x is Tuple) echo "Tuple\n";

  $x = __hhvm_intrinsics\launder_value($x);
  if ($x is Shape) echo "Shape\n";
  if ($x is Tuple) echo "Tuple\n";

  $x = tuple('a', 123);
  if ($x is Shape) echo "Shape\n";
  if ($x is Tuple) echo "Tuple\n";

  $x = __hhvm_intrinsics\launder_value($x);
  if ($x is Shape) echo "Shape\n";
  if ($x is Tuple) echo "Tuple\n";

  echo "Done\n";
}
