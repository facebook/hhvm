<?hh
// (c) Meta Platforms, Inc. and affiliates. All Rights Reserved.

async function makeString():Awaitable<mixed> {
  return 3;
}

<<__EntryPoint>>
async function main():Awaitable<void> {
  $s = HH\FIXME\UNSAFE_CAST<mixed,string>(await makeString());
}
