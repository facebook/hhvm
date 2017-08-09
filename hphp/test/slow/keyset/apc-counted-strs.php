<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function make_counted_str($s) {
  return __hhvm_intrinsics\launder_value($s) . '---' . __hhvm_intrinsics\launder_value($s);
}

function main() {
  $ks = apc_fetch('keyset-key');
  if ($ks === false) {
    $ks = keyset[
      make_counted_str('val1'),
      make_counted_str('val2'),
      make_counted_str('val3'),
      make_counted_str('val3')
    ];
    apc_store('keyset-key', $ks);
  }
  return $ks;
}

var_dump(array_keys(main()));
