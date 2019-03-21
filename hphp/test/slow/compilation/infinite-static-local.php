<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  static $v = vec[];
  for ($i = 0; $i < __hhvm_intrinsics\launder_value(3); $i++) $v[] = $v;
  return $v;
}

<<__EntryPoint>>
function main_infinite_static_local() {
var_dump(main());
}
