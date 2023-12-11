<?hh

async function block() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function bar($id) :Awaitable<mixed>{
  await block();
  return $id * $id;
}

async function foo() :Awaitable<mixed>{
  await block();
  return await \HH\Asio\m(
    array_map(
      async $id ==> {
        return await bar($id);
      },
      vec[1,2,3,4],
    )
  );
}


<<__EntryPoint>>
function main_lambda3() :mixed{
var_dump(HH\Asio\join(foo()));
}
