<?hh

async function block() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function foo($a) :Awaitable<mixed>{
  $fn = async ($b, $c) ==> {
    await block();
    return $a * $b + $c;
  };

  return await $fn(47, 26);
}


<<__EntryPoint>>
function main_lambda4() :mixed{
var_dump(HH\Asio\join(foo(42)));
}
