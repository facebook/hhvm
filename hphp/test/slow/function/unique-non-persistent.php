<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

if (apc_fetch('included') === false) {
  include 'unique-non-persistent.inc';
}
apc_store('included', true);

function foo() { bar(); }
foo();
