<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

require __DIR__ . "/../async/gen-stubs.inc";

async function f() { return 1; }

async function test() {
  $x = [f(), f(), f()];
  await gena($x);
}
HH\Asio\join(test());
echo "DONE\n";
