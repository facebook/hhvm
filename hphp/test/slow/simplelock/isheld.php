<?hh

async function thread_func() {
  echo "thread\n";

  await HH\SimpleLock\lock('thread_lock1');
  await HH\SimpleLock\lock('thread_lock2');
  await HH\SimpleLock\lock('end_lock');

  echo 'thread: '; var_dump(HH\SimpleLock\is_held('main_lock1'));
  echo 'thread: '; var_dump(HH\SimpleLock\is_held('main_lock2'));
  echo 'thread: '; var_dump(HH\SimpleLock\is_held('thread_lock1'));
  echo 'thread: '; var_dump(HH\SimpleLock\is_held('thread_lock2'));

  $_ = true;
  $t = apc_inc('threads', 1, inout $_);

  await HH\SimpleLock\lock('sync');
  HH\SimpleLock\unlock('thread_lock2');
}

function thread_main() {
  HH\Asio\join(thread_func());
}

<<__EntryPoint>>
async function main() {
  if (HH\execution_context() === "xbox") return;

  await HH\SimpleLock\lock('main_lock1');
  await HH\SimpleLock\lock('main_lock2');
  await HH\SimpleLock\lock('sync');
  apc_store('threads', 0);

  $func = fb_call_user_func_async(
    __FILE__,
    'thread_main'
  );

  $_ = true;
  while (apc_fetch('threads', inout $_) !== 1) usleep(10);

  echo 'main: '; var_dump(HH\SimpleLock\is_held('main_lock1'));
  echo 'main: '; var_dump(HH\SimpleLock\is_held('main_lock2'));
  echo 'main: '; var_dump(HH\SimpleLock\is_held('thread_lock1'));
  echo 'main: '; var_dump(HH\SimpleLock\is_held('thread_lock2'));

  HH\SimpleLock\unlock('sync');
  fb_end_user_func_async($func);

  HH\SimpleLock\unlock('main_lock2');
  await HH\SimpleLock\lock('end_lock');

  echo 'main: '; var_dump(HH\SimpleLock\is_held('main_lock1'));
  echo 'main: '; var_dump(HH\SimpleLock\is_held('main_lock2'));
  echo 'main: '; var_dump(HH\SimpleLock\is_held('thread_lock1'));
  echo 'main: '; var_dump(HH\SimpleLock\is_held('thread_lock2'));

  echo "Main done.\n";
}

