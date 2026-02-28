<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function make_counted_str($s) :mixed{
  return __hhvm_intrinsics\launder_value($s) . '---' . __hhvm_intrinsics\launder_value($s);
}

function main() :mixed{
  $ks = __hhvm_intrinsics\apc_fetch_no_check('keyset-key');
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


<<__EntryPoint>>
function main_apc_counted_strs() :mixed{
var_dump(array_keys(main()));
}
