<?hh

async function thread_func() {
  echo "thread\n";

  $_ = true;
  $t = apc_inc('threads', 1, inout $_);

  await HH\SimpleLock\lock("sync");

  $t = apc_inc('counter', 1, inout $_);
  for ($i = 0; $i < 10; $i++) {
    echo "thread($t): $i\n";
    await SleepWaitHandle::create(1000);
  }

  if ($t % 2) HH\SimpleLock\unlock("sync");
}

function thread_main() {
  HH\Asio\join(thread_func());
}

<<__EntryPoint>>
async function main() {
  if (HH\execution_context() === "xbox") return;

  await HH\SimpleLock\lock("sync");
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

  echo "Main done.\n";
}
