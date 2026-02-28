<?hh

async function block() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function foo() :Awaitable<mixed>{
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


<<__EntryPoint>>
function main_immed_lambda1() :mixed{
var_dump(HH\Asio\join(foo()));
}
