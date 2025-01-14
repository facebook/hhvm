<?hh

function thread_main() {
  HH\SimpleLock\lock('abandon');
  HH\SimpleLock\lock('abandon');
  HH\SimpleLock\lock('abandon');
  HH\SimpleLock\lock('abandon');
  HH\SimpleLock\lock('abandon');

  $_ = true;
  $t = apc_inc('threads', 1, inout $_);
}

<<__EntryPoint>>
async function main() {
  if (HH\execution_context() === "xbox") return;

  await HH\SimpleLock\lock('abandon');
  apc_store('threads', 0);

  $funcs = vec[];
  for ($i = 0; $i < 4; $i++) {
    $funcs[] = fb_call_user_func_async(
      __FILE__,
      'thread_main'
    );
  }

  $_ = true;
  while (apc_fetch('threads', inout $_) !== count($funcs)) usleep(10);

  HH\SimpleLock\unlock('abandon');
  foreach ($funcs as $f) fb_end_user_func_async($f);

  await HH\SimpleLock\lock('abandon');
  echo "Main done.\n";
}
