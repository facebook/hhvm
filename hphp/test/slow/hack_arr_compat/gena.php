<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function f() { return 1; }

async function test() {
  $x = [f(), f(), f()];
  await gena($x);
}

<<__EntryPoint>>
function main_gena() {
HH\Asio\join(test());
echo "DONE\n";
}
