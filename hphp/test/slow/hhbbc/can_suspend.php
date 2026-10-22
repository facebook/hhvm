<?hh

use namespace HH\Lib\Dict;
use namespace HH\Lib\Vec;

async function eager_leaf(int $value): Awaitable<int> {
  return $value + 1;
}

async function eager_chain(int $value): Awaitable<int> {
  return await eager_leaf($value);
}

async function suspending_leaf(int $value): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  return $value + 1;
}

async function suspending_chain(int $value): Awaitable<int> {
  return await suspending_leaf($value);
}

async function apply_async(
  (function(): Awaitable<int>) $callback,
): Awaitable<int> {
  return await $callback();
}

async function conditional_apply_async(
  bool $suspend,
  (function(): Awaitable<int>) $callback,
): Awaitable<int> {
  if ($suspend) await HH\Asio\later();
  return await $callback();
}

async function eager_closure(): Awaitable<int> {
  return await apply_async(async () ==> 10);
}

async function suspending_closure(): Awaitable<int> {
  return await apply_async(async () ==> {
    await RescheduleWaitHandle::create(0, 0);
    return 11;
  });
}

async function eager_conditional_closure(): Awaitable<int> {
  return await conditional_apply_async(false, async () ==> 12);
}

async function eager_all(): Awaitable<void> {
  await HH\AwaitAllWaitHandle::fromVec(
    vec[eager_leaf(1), eager_leaf(2)],
  );
}

async function suspending_all(): Awaitable<void> {
  await HH\AwaitAllWaitHandle::fromVec(
    vec[eager_leaf(1), suspending_leaf(2)],
  );
}

async function eager_map(vec<int> $values): Awaitable<vec<int>> {
  return await Vec\map_async(
    $values,
    async $value ==> $value + 1,
  );
}

async function suspending_map(): Awaitable<vec<int>> {
  return await Vec\map_async(
    vec[1, 2],
    async $value ==> {
      await RescheduleWaitHandle::create(0, 0);
      return $value + 1;
    },
  );
}

async function eager_dict_map(
  dict<int, int> $values,
): Awaitable<dict<int, int>> {
  return await Dict\map_with_key_async(
    $values,
    async ($key, $value) ==> 2 * $key + $value,
  );
}

<<__EntryPoint>>
function main(): void {
  var_dump(HH\Asio\join(eager_leaf(1)));
  var_dump(HH\Asio\join(eager_chain(2)));
  var_dump(HH\Asio\join(eager_closure()));
  var_dump(HH\Asio\join(suspending_closure()));
  var_dump(HH\Asio\join(eager_conditional_closure()));
  HH\Asio\join(eager_all());
  echo "eager all done\n";
  HH\Asio\join(suspending_all());
  echo "suspending all done\n";
  var_dump(HH\Asio\join(eager_map(vec[1, 2])));
  var_dump(HH\Asio\join(suspending_map()));
  var_dump(HH\Asio\join(eager_dict_map(dict[3 => 1, 2 => 3])));
  var_dump(HH\Asio\join(suspending_chain(12)));
}
