<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

$obj = Map{'1' => 123, 1 => 123};
$obj['1'] = $obj;

apc_store('some-key', vec[$obj]);
var_dump(apc_fetch('some-key'));
