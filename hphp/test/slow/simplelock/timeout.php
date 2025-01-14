<?hh

const US_IN_MS = 1000;

async function thread_func() {
  echo "thread\n";

  $_ = true;
  $t = apc_inc('threads', 1, inout $_);

  await HH\SimpleLock\lock("sync");
  HH\SimpleLock\unlock("sync");

  try {
    await HH\SimpleLock\lock_with_timeout('hold', 100);
  } catch (Exception $ex) {
    echo $ex->getMessage()."\n";
  }
}

async function timeout($time) {
  try {
    await HH\SimpleLock\lock_with_timeout('hold', $time * US_IN_MS);
  } catch (Exception $_) {
    echo "Timed out: $time\n";
    return;
  }
  echo "Got lock\n";
}

async function multi() {
  concurrent {
    await timeout(100);
    await timeout(150);
    await timeout(200);
    await timeout(250);
  }
}

function thread_main() {
  HH\Asio\join(thread_func());
}

<<__EntryPoint>>
async function main() {
  if (HH\execution_context() === "xbox") return;

  await HH\SimpleLock\lock("sync");
  await HH\SimpleLock\lock("hold");
  apc_store('counter', 0);
  apc_store('threads', 0);

  $funcs = vec[];
  for ($i = 0; $i < 4; $i++) {
    $funcs[] = fb_call_user_func_async(
      __FILE__,
      'thread_main'
    );
  }

  $_ = true;
  while (apc_fetch('threads', inout $_) !== 4) usleep(10);

  HH\SimpleLock\unlock("sync");
  foreach ($funcs as $f) fb_end_user_func_async($f);

  await multi();

  concurrent {
    await async {
      await timeout(1000000);
    };
    await async {
      await SleepWaitHandle::create(1000);
      HH\SimpleLock\unlock('hold');
    };
  }

  echo "Main done.\n";
}
