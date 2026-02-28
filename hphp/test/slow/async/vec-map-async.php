<?hh

use namespace HH\Lib\Vec;

async function eager(int $val): Awaitable<int> {
  return $val + 1;
}

async function suspend(int $val): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  return $val + 1;
}

<<__EntryPoint>>
async function main() {
  var_dump(await Vec\map_async(vec[], eager<>));
  var_dump(await Vec\map_async(vec[], suspend<>));
  var_dump(await Vec\map_async(vec[1, 2, 3], eager<>));
  var_dump(await Vec\map_async(vec[1, 2, 3], suspend<>));
}
