<?hh

function thread_main() {
  HH\Asio\join(HH\SimpleLock\lock("x"));
  HH\Asio\join(HH\SimpleLock\lock("hello"));
  echo "Thread start.\n";
  apc_store(__FILE__.'_thread', 1);
  HH\Asio\join(HH\SimpleLock\lock("goodbye"));
  echo "Thread done.\n";
}

<<__EntryPoint>>
async function main() {
  if (HH\execution_context() === "xbox") return;

  await HH\SimpleLock\lock("hello");
  await HH\SimpleLock\lock("goodbye");

  HH\SimpleLock\unlock("hello");

  apc_store(__FILE__.'_thread', 0);
  $hnd = fb_call_user_func_async(
    __FILE__,
    'thread_main'
  );

  while (__hhvm_intrinsics\apc_fetch_no_check(__FILE__.'_thread') !== 1) {
    usleep(500);
  }

  echo "Main sleep.\n";
  usleep(500);
  HH\SimpleLock\unlock("goodbye");
  await HH\SimpleLock\lock("x");
  echo "Main done.\n";
}
