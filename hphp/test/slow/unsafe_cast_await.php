<?hh
// (c) Meta Platforms, Inc. and affiliates. All Rights Reserved.

async function makeString():Awaitable<mixed> {
  return "3";
}

function expectString(string $s):void { }

<<__EntryPoint>>
async function main():Awaitable<void> {
  $s1 = (await makeString()) as string;
  expectString($s1);
  $s2 = HH\FIXME\UNSAFE_CAST<mixed,string>(await makeString());
  expectString($s2);
  // This should also work
  $s3 = await HH\FIXME\UNSAFE_CAST<Awaitable<mixed>,Awaitable<string>>(makeString());
  expectString($s3);
  echo "Done.\n";
}
