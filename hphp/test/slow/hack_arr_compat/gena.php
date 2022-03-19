<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function gena<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<dict<Tk, Tv>> {
  $awaitables = dict($awaitables);
  await AwaitAllWaitHandle::fromDict($awaitables);
  foreach ($awaitables as $index => $value) {
    $awaitables[$index] = HH\Asio\result($value);
  }
  return $awaitables;
}

async function f() { return 1; }

async function test() {
  $x = vec[f(), f(), f()];
  await gena($x);
}

<<__EntryPoint>>
function main_gena() {
  HH\Asio\join(test());
  echo "DONE\n";
}
