<?hh

function foo() { bar(); }

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_unique_non_persistent() {
if (__hhvm_intrinsics\apc_fetch_no_check('included') === false) {
  include 'unique-non-persistent.inc';
}
apc_store('included', true);
foo();
}
