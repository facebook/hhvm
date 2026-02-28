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
  concurrent {
    await fb_gen_user_func_array(__FILE__, 'thread1', vec[$lockname]);
    await fb_gen_user_func_array(__FILE__, 'thread2', vec[$lockname]);
  }
}

<<__DynamicallyCallable>>
function thread1(string $lockname) {
  thread1LockingMethod($lockname);
  sleep(3);
  echo "T1 ended...\n";
}

function thread1LockingMethod(string $lockname) {
  $lock = HH\XReqSync::get($lockname);
  echo "T1 getting lock...\n";
  var_dump(HH\Asio\join($lock->genLock()));
  echo "T1 got lock, sleeping 3s...\n";
  sleep(3);
  echo "T1 scope ending without releasing lock...\n";

  // Note that this is testing that the current behavior is that scope exit
  // is releasing the lock. The exact definition of scope exit should remain
  // not strictly defined and the main usage of this API should always be with
  // be with "unlock". If only this test fails and because of scope definition
  // change, it should be reasonable to change the test.
}

<<__DynamicallyCallable>>
function thread2(string $lockname) {
  $lock = HH\XReqSync::get($lockname);
  sleep(1);
  echo "T2 trying to get lock...\n";
  var_dump(HH\Asio\join($lock->genLock()));
  echo "T2 got lock.\n";
  $lock->unlock();
}
