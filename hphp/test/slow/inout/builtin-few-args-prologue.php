<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main() :mixed{
  $a = __hhvm_intrinsics\launder_value(123);
  $b = __hhvm_intrinsics\launder_value(456);
  $c = __hhvm_intrinsics\launder_value(789);
  try { __hhvm_intrinsics\dummy_lots_inout(); } catch (Exception $e) { var_dump($e->getMessage()); }
  var_dump($a, $b, $c);
}
