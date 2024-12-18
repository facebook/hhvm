<?hh

<<__EntryPoint>>
function main(): void {
  $ctx = HH\execution_context();
  if ($ctx === 'xbox') {
    return;
  }

  HH\Asio\join(genMain());
}

async function genMain(): Awaitable<void> {
  $lockname = 'lock'.time().rand();
  $lock = HH\XReqSync::get($lockname);
  await $lock->genLock();
  await fb_gen_user_func_array(__FILE__, 'thread1', vec[$lockname]);
}

<<__DynamicallyCallable>>
function thread1(string $lockname) {
  HH\Asio\join(genMultiUnlock($lockname));
}

async function genMultiUnlock(string $lockname): Awaitable<void> {
  $lock = HH\XReqSync::get($lockname);
  echo "T1 concurrently trying multiple genLocks...\n";
  concurrent {
    await genUnlock($lock, 2500);
    await genUnlock($lock, 2000);
    await genUnlock($lock, 500);
    await genUnlock($lock, 1000);
    await genUnlock($lock, 100);
    await genUnlock($lock, 1500);
  }
}

async function genUnlock(HH\XReqSync $lock, int $time): Awaitable<void> {
  await $lock->genLock($time);
  echo "genLock with $time returned\n";
}
