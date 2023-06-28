<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function get_count() :mixed{
  $count = __hhvm_intrinsics\apc_fetch_no_check('count');
  if ($count === false) {
    $count = 0;
    apc_store('count', $count);
  }
  return $count;
}

function store() :mixed{
  echo "store()\n";
  $a = new A();
  $a->x = true;
  apc_store('some-key', $a);
  __hhvm_intrinsics\apc_fetch_no_check('some-key');

  $b = new B();
  $b->x = true;
  apc_store('some-key2', $b);
  __hhvm_intrinsics\apc_fetch_no_check('some-key2');
}

function get() :mixed{
  echo "get()\n";
  __hhvm_intrinsics\apc_fetch_no_check('some-key');
  __hhvm_intrinsics\apc_fetch_no_check('some-key2');
}

function run() :mixed{
  $count = get_count();
  echo "Count: $count\n";
  if ($count == 0) {
    store();
  } else {
    get();
  }
  apc_store('count', $count+1);
}
<<__EntryPoint>>
function entrypoint_apc(): void {

  if ((get_count() % 2) == 0) {
    require 'apc2.inc';
  } else {
    require 'apc3.inc';
  }

  require 'apc1.inc';

  run();
}
