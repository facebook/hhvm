<?hh

async function test_cancel_pending(): Awaitable<void> {
  $wh = SleepWaitHandle::create(5_000_000); // 5 seconds

  $result = HH\Asio\cancel_sleep_nothrow($wh);
  echo "cancel pending: ".($result ? "true" : "false")."\n";

  $value = await $wh;
  echo "await result is null: ".($value === null ? "true" : "false")."\n";
}

async function test_cancel_already_finished(): Awaitable<void> {
  $wh = SleepWaitHandle::create(0);
  await $wh;

  $result = HH\Asio\cancel_sleep_nothrow($wh);
  echo "cancel finished: ".($result ? "true" : "false")."\n";
}

async function test_cancel_non_sleep(): Awaitable<void> {
  HH\Asio\cancel_sleep_nothrow(
    ConditionWaitHandle::create(SleepWaitHandle::create(0))
  );
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  await test_cancel_pending();
  await test_cancel_already_finished();
  await test_cancel_non_sleep();
}
