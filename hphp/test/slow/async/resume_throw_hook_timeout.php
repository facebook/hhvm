<?hh

<<__EntryPoint>>
async function test() :Awaitable<mixed>{
  $failed_awaitable = async {
    await RescheduleWaitHandle::create(0, 0);
    throw new Exception();
  };

  $blocked_awaitables = vec[];
  for ($i = 0; $i < 100000; ++$i) {
    $blocked_awaitables[] = async {
      try {
        await $failed_awaitable;
      } catch (Exception $e) {
      }
    };
  }

  $all_blocked = AwaitAllWaitHandle::fromVec($blocked_awaitables);

  register_postsend_function(() ==> {
    set_time_limit(0);

    // Hopefully all awaitables are in consistent state and we don't crash.
    HH\Asio\join($all_blocked);
    echo "Done!\n";
  });

  set_time_limit(1);
  usleep(990000);

  // Hopefully the timeout happens here while we process awaitables.
  await $all_blocked;

  // But if not, make sure we timeout to prevent test flakiness.
  while (true);
}
