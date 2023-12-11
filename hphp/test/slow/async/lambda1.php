<?hh

async function block() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function foo() :Awaitable<mixed>{
  await block();
  return await \HH\Asio\m(
    array_map(
      async $id ==> $id * $id,
      vec[1,2,3,4],
    )
  );
}


<<__EntryPoint>>
function main_lambda1() :mixed{
var_dump(HH\Asio\join(foo()));
}
