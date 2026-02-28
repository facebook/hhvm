<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_recursive_collection() :mixed{
$obj = Map{'1' => 123, 1 => 123};
$obj['1'] = $obj;

apc_store('some-key', vec[$obj]);
var_dump(__hhvm_intrinsics\apc_fetch_no_check('some-key'));
}
