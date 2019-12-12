<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test(dict $x) {
  if (sizeof($x ?: dict[]) == 0) {
    __hhvm_intrinsics\launder_value(false);
  }
}

<<__EntryPoint>>
function main() {
  test(__hhvm_intrinsics\launder_value(dict[]));
  echo "DONE\n";
}
