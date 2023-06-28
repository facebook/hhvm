<?hh

function block() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function foo() :Awaitable<mixed>{
  return await null;
}

async function foob() :Awaitable<mixed>{
  await block();
  return await null;
}

async function test() :Awaitable<mixed>{
  $a = await null;
  var_dump($a);

  $a = await foo();
  var_dump($a);

  $a = await foob();
  var_dump($a);
}


<<__EntryPoint>>
function main_null() :mixed{
;

HH\Asio\join(test());
}
