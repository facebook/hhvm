<?hh

<<__EntryPoint>>
function main(): void {
  $ctx = HH\execution_context();
  if ($ctx === 'xbox') {
    return;
  }

  HH\Asio\join(genMain());
}

function getAPCKey(string $lockname) {
  return $lockname.'_count';
}

async function reportProgress(string $lockname): Awaitable<void> {
  $apc_key = getAPCKey($lockname);

  // For up to 20 seconds at 0.1 intervals
  $last_finished = 0;
  for ($i = 0; $i < 200; $i++) {
    $success = false;
    $count = apc_fetch($apc_key, inout $success);
    if ($count !== $last_finished) {
      $last_finished = $count;
      echo "Threads finished: $count\n";
      if ($count == 8) {
        $seconds = $i * 0.1;
        if ($seconds > 15) {
          echo "Overall time over 15s\n";
        } else {
          echo "Overall time under 15s\n";
        }
        return;
      }
    }
    // 100ms
    await SleepWaitHandle::create(1000 * 100);
  }
}

async function genMain(): Awaitable<void> {
  $lockname = 'lock'.time().rand();
  $apc_key = getAPCKey($lockname);
  apc_store($apc_key, 0);

  // 8 Threads, 1s each. Should take 8s to complete.
  concurrent {
    await fb_gen_user_func_array(__FILE__, 'thread', vec[$lockname]);
    await fb_gen_user_func_array(__FILE__, 'thread', vec[$lockname]);
    await fb_gen_user_func_array(__FILE__, 'thread', vec[$lockname]);
    await fb_gen_user_func_array(__FILE__, 'thread', vec[$lockname]);
    await fb_gen_user_func_array(__FILE__, 'thread', vec[$lockname]);
    await fb_gen_user_func_array(__FILE__, 'thread', vec[$lockname]);
    await fb_gen_user_func_array(__FILE__, 'thread', vec[$lockname]);
    await fb_gen_user_func_array(__FILE__, 'thread', vec[$lockname]);
    await reportProgress($lockname);
  }
}

<<__DynamicallyCallable>>
function thread(string $lockname) {
  echo "Thread started\n";
  usleep(1000 * 500);
  $lock = HH\XReqSync::get($lockname);
  HH\Asio\join($lock->genLock());
  echo "Thread got lock\n";
  sleep(1);
  echo "Thread releasing lock\n";
  $lock->unlock();
  $success = false;
  apc_inc(getAPCKey($lockname), 1, inout $success);
}
