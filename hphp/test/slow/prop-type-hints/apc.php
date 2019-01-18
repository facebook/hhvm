<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function get_count() {
  $count = apc_fetch('count');
  if ($count === false) {
    $count = 0;
    apc_store('count', $count);
  }
  return $count;
}

if ((get_count() % 2) == 0) {
  require 'apc2.inc';
} else {
  require 'apc3.inc';
}

require 'apc1.inc';

function store() {
  echo "store()\n";
  $a = new A();
  $a->x = true;
  apc_store('some-key', $a);
  apc_fetch('some-key');

  $b = new B();
  $b->x = true;
  apc_store('some-key2', $b);
  apc_fetch('some-key2');
}

function get() {
  echo "get()\n";
  apc_fetch('some-key');
  apc_fetch('some-key2');
}

function run() {
  $count = get_count();
  echo "Count: $count\n";
  if ($count == 0) {
    store();
  } else {
    get();
  }
  apc_store('count', $count+1);
}

run();
