<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

$count = apc_fetch('count');
if ($count === false) $count = 0;
if ($count < 3) {
  function bar() { echo "bar called....\n"; }
}
++$count;
apc_store('count', $count);

function foo() { bar(); }
foo();
