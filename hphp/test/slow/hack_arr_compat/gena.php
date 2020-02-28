<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function gena<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<darray<Tk, Tv>> {
  $awaitables = darray($awaitables);
  await AwaitAllWaitHandle::fromDArray($awaitables);
  foreach ($awaitables as $index => $value) {
    $awaitables[$index] = HH\Asio\result($value);
  }
  return $awaitables;
}

async function f() { return 1; }

async function test() {
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[f(), f(), f()]);
  await gena($x);
}

<<__EntryPoint>>
function main_gena() {
HH\Asio\join(test());
echo "DONE\n";
}
