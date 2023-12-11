<?hh

<<__EntryPoint>>
function main(): void {

  if (HH\execution_context() === "xbox") return;

  $a = vec[];
  $a[] = fb_gen_user_func_array(
    __FILE__,
    'test',
    vec[ 0 ],
  );
  $a[] = fb_gen_user_func_array(
    __FILE__,
    'test',
    vec[ 1 ],
  );
  $a[] = fb_gen_user_func_array(
    __FILE__,
    'test',
    vec[ 2 ],
  );
  $a[] = fb_gen_user_func_array(
    __FILE__,
    'test',
    vec[ 3 ],
  );
  HH\Asio\join(AwaitAllWaitHandle::fromVec($a));
}

function test(int $it) :mixed{
  $ok = false;
  switch ($it) {
    case 0:
      // add with ttl 1.
      apc_store('foo', 'bar', 1);
      // add with ttl 5. does not update expire queue
      apc_store('foo', 'baz', 5);
      echo "Added. Sleeping for 2s\n";
      sleep(2);
      // purgeExpired will run on requestExit, will pop foo's entry,
      // but not kill it, because it didn't exipre yet.
      return;
    case 1:
      // sleep until it really expires
      sleep(6);
      // should do a deferred expire
      $val = apc_fetch('foo', inout $ok);
      var_dump($val);
      // stay alive until after case 2 runs, so we don't purgeExpired
      sleep(2);
      return;
    case 2:
      sleep(7);
      // Not the original thread, so sees the old value
      $val = apc_fetch('foo', inout $ok);
      var_dump($val);
      return;
    case 3:
      sleep(9);
      // the thread that did the deferred expire has exited, so the
      // value should be gone now.
      $val = apc_fetch('foo', inout $ok);
      var_dump($val);
      return;
  }
}
