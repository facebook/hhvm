<?hh

use namespace HH\Lib\Dict;

async function eager(int $val): Awaitable<int> {
  return $val + 1;
}

async function suspend(int $val): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  return $val + 1;
}

<<__EntryPoint>>
async function main() {
  var_dump(await Dict\map_async(dict[], eager<>));
  var_dump(await Dict\map_async(dict[], suspend<>));
  var_dump(await Dict\map_async(dict[3 => 1, 2 => 3, 1 => 2], eager<>));
  var_dump(await Dict\map_async(dict[3 => 1, 2 => 3, 1 => 2], suspend<>));
}
