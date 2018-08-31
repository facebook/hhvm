<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_recursive_collection() {
$obj = Map{'1' => 123, 1 => 123};
$obj['1'] = $obj;

apc_store('some-key', vec[$obj]);
var_dump(apc_fetch('some-key'));
}
