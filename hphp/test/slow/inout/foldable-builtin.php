<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__ALWAYS_INLINE>>
function inline($a, $b, $c, $d) :mixed{
  $a = __hhvm_intrinsics\builtin_io_foldable($a, inout $b, inout $c, inout $d);
  var_dump($a, $b, $c, $d);
}

function call_inline() :mixed{
  inline(2, 3, 5, 7);
}

<<__EntryPoint>>
function main() :mixed{
  $a = 2;
  $b = 3;
  $c = 5;
  $d = 7;
  $a = __hhvm_intrinsics\builtin_io_foldable($a, inout $b, inout $c, inout $d);
  var_dump($a, $b, $c, $d);

  call_inline();
  call_inline();
}
