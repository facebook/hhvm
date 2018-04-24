<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function f() {
  return 42;
}

async function test() {
  $x = varray[f(), f(), f()];
  await AwaitAllWaitHandle::fromVArray($x);
}

HH\Asio\join(test());
echo "DONE\n";
