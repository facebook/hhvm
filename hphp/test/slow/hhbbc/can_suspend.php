<?hh

use namespace HH\Lib\Dict;
use namespace HH\Lib\Vec;

async function eager_leaf(int $value): Awaitable<int> {
  return $value + 1;
}

async function eager_chain(int $value): Awaitable<int> {
  return await eager_leaf($value);
}

async function eager_stored(int $value): Awaitable<int> {
  $handle = eager_leaf($value);
  return await $handle;
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

async function eager_concurrent(): Awaitable<(int, int)> {
  concurrent {
    $first = await eager_leaf(20);
    $second = await eager_leaf(30);
  }
  return tuple($first, $second);
}

async function mixed_concurrent(): Awaitable<(int, int, int)> {
  concurrent {
    $eager = await eager_leaf(50);
    $first = await suspending_leaf(40);
    $last = await suspending_leaf(60);
  }
  return tuple($first, $eager, $last);
}

final class CompletionState {
  public bool $first = false;
  public bool $last = false;
}

async function mark_after_suspend(
  CompletionState $state,
  bool $first,
): Awaitable<void> {
  await RescheduleWaitHandle::create(0, 0);
  if ($first) {
    $state->first = true;
  } else {
    $state->last = true;
  }
}

async function eager_failure(): Awaitable<void> {
  throw new Exception('eager failure');
}

async function caught_eager_failure(): Awaitable<string> {
  $handle = eager_failure();
  try {
    await $handle;
  } catch (Exception $exception) {
    return $exception->getMessage();
  }
  return 'unexpected success';
}

async function catch_direct_eager_failure(): Awaitable<string> {
  try {
    await eager_failure();
  } catch (Exception $exception) {
    return 'direct '.$exception->getMessage();
  }
  return 'unexpected success';
}

// Removing the eager failure from AwaitAll must still let both pending
// siblings finish before WHResult rethrows it.
async function mixed_concurrent_failure(): Awaitable<(bool, bool)> {
  $state = new CompletionState();
  try {
    concurrent {
      await eager_failure();
      await mark_after_suspend($state, true);
      await mark_after_suspend($state, false);
    }
  } catch (Exception $_) {
  }
  return tuple($state->first, $state->last);
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

<<__Memoize>>
async function eager_memo_throws(): Awaitable<int> {
  throw new Exception('memo');
}

async function catch_eager_memo_throw(): Awaitable<string> {
  try {
    await eager_memo_throws();
    return 'unexpected';
  } catch (Exception $exception) {
    return $exception->getMessage();
  }
}

<<__EntryPoint>>
function main(): void {
  var_dump(HH\Asio\join(eager_leaf(1)));
  var_dump(HH\Asio\join(eager_chain(2)));
  var_dump(HH\Asio\join(eager_stored(70)));
  var_dump(HH\Asio\join(eager_closure()));
  var_dump(HH\Asio\join(suspending_closure()));
  var_dump(HH\Asio\join(eager_conditional_closure()));
  HH\Asio\join(eager_all());
  echo "eager all done\n";
  HH\Asio\join(suspending_all());
  echo "suspending all done\n";
  var_dump(HH\Asio\join(eager_concurrent()));
  var_dump(HH\Asio\join(mixed_concurrent()));
  var_dump(HH\Asio\join(caught_eager_failure()));
  var_dump(HH\Asio\join(catch_direct_eager_failure()));
  var_dump(HH\Asio\join(mixed_concurrent_failure()));
  var_dump(HH\Asio\join(eager_map(vec[1, 2])));
  var_dump(HH\Asio\join(suspending_map()));
  var_dump(HH\Asio\join(eager_dict_map(dict[3 => 1, 2 => 3])));
  var_dump(HH\Asio\join(catch_eager_memo_throw()));
  var_dump(HH\Asio\join(suspending_chain(12)));
}
