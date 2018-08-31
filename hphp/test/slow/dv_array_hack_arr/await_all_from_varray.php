<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function f() {
  return 42;
}

async function test() {
  $x = varray[f(), f(), f()];
  await AwaitAllWaitHandle::fromVArray($x);
}


<<__EntryPoint>>
function main_await_all_from_varray() {
HH\Asio\join(test());
echo "DONE\n";
}
