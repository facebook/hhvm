<?hh

class Ref {
  function __construct(public $value)[] {}
}

async function reschedule() {
  return await RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}

async function answer() {
  await reschedule();
  return 42;
}

async function test() {
  concurrent {
    $a = await answer();
    $b = await answer();
    $c = await answer();
  }
  return $a;
}

<<__EntryPoint>>
async function main_await_all_callback() {
  $join_it = new Ref(0);

  ConcurrentWaitHandle::setOnCreateCallback(($a, $b) ==> {
    echo "onCreateCallback()...\n";
    if ($join_it->value === 1) HH\Asio\join($a);
    if ($join_it->value === 2) exit(0);
  });

  await test();
  ++$join_it->value;
  await test();
  ++$join_it->value;
  await test();
}
