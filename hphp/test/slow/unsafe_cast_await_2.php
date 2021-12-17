<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

async function gen_int1(Awaitable<mixed> $i): Awaitable<int> {
  return await $i;
}

async function gen_int2(Awaitable<mixed> $i): Awaitable<int> {
  return HH\FIXME\UNSAFE_CAST<mixed, int>(await $i);
}

<<__EntryPoint>>
async function main():Awaitable<void> {
  $i1 = await gen_int1(async { return 5; });
  var_dump($i1);
  $i2 = await gen_int2(async { return 6; });
  var_dump($i2);
}
