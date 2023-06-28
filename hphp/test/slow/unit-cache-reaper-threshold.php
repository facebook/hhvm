<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main() :mixed{
  $count = __hhvm_intrinsics\apc_fetch_no_check("count");
  if ($count === false) $count = 0;

  if ($count == 0) {
    require_once 'unit-cache-reaper-1.inc';
    require_once 'unit-cache-reaper-2.inc';
    foo();
    bar();
    var_dump(__hhvm_intrinsics\is_unit_loaded(__DIR__ . '/unit-cache-reaper-1.inc'));
    var_dump(__hhvm_intrinsics\is_unit_loaded(__DIR__ . '/unit-cache-reaper-2.inc'));
  } else {
    sleep(15);
    var_dump(__hhvm_intrinsics\is_unit_loaded(__DIR__ . '/unit-cache-reaper-1.inc'));
    var_dump(__hhvm_intrinsics\is_unit_loaded(__DIR__ . '/unit-cache-reaper-2.inc'));
  }

  apc_store("count", $count+1);
}
