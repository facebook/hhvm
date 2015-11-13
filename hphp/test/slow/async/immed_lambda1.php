<?hh

async function block() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function foo() {
  $awaitable = async {
    echo "waiting\n";
    await block();
    echo "ready\n";
    return 42;
  };
  var_dump(gettype($awaitable));
  var_dump(get_class($awaitable));
  return await $awaitable;
}

var_dump(HH\Asio\join(foo()));
