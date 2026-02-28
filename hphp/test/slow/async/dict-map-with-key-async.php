<?hh

use namespace HH\Lib\Dict;

async function eager(int $key, int $val): Awaitable<int> {
  return 2 * $key + $val + 1;
}

async function suspend(int $key, int $val): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  return 2 * $key + $val + 1;
}

<<__EntryPoint>>
async function main() {
  var_dump(await Dict\map_with_key_async(dict[], eager<>));
  var_dump(await Dict\map_with_key_async(dict[], suspend<>));
  var_dump(await Dict\map_with_key_async(dict[3 => 1, 2 => 3, 1 => 2], eager<>));
  var_dump(await Dict\map_with_key_async(dict[3 => 1, 2 => 3, 1 => 2], suspend<>));
}
