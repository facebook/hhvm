<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function byref(&$x) {}
function build() {
  $d = dict[];
  $k = __hhvm_intrinsics\launder_value('key');
  while (__hhvm_intrinsics\launder_value(false)) {
    $d[$k] = $d[$k] ?? dict[];
    $d[$k] = $d[$k] ?? dict[];
    byref(&$d[$k]['k2']);
  }
  return $d;
}

var_dump(build());
