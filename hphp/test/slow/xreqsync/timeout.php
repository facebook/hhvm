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
    await fb_gen_user_func_array(__FILE__, 'thread3', vec[$lockname]);
  }
}

<<__DynamicallyCallable>>
function thread1(string $lockname) {
  $lock = HH\XReqSync::get($lockname);
  echo "T1 getting lock...\n";
  var_dump(HH\Asio\join($lock->genLock()));
  echo "T1 got lock, sleeping 3 seconds...\n";
  sleep(3);
  echo "T1 releasing lock...\n";
  $lock->unlock();
}

<<__DynamicallyCallable>>
function thread2(string $lockname) {
  $lock = HH\XReqSync::get($lockname);
  sleep(1);
  echo "T2 trying to get lock with a too short timeout...\n";
  var_dump(HH\Asio\join($lock->genLock(10)));
  echo "T2 failed to get the lock after 10ms...\n";
}

<<__DynamicallyCallable>>
function thread3(string $lockname) {
  $lock = HH\XReqSync::get($lockname);
  sleep(2);
  echo "T3 trying to get lock with a too long timeout...\n";
  $ms_before = (int)microtime(true)*1000;
  var_dump(HH\Asio\join($lock->genLock(5000)));
  $ms_after = (int)microtime(true)*1000;
  if ($ms_after - $ms_before < 4000) {
    echo "T3 got lock in less than timeout.\n";
  } else {
    echo "T3 got lock too slow (error).\n";
  }

  $lock->unlock();
}
