<?hh

function block() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function f() :Awaitable<mixed>{
  return 1;
}

async function foo() :Awaitable<mixed>{
  $f = f();
  $a = await $f;
  var_dump($a);
  await block();
  $f = f();
  $a = await $f;
  var_dump($a);
}

<<__EntryPoint>>
function main_awaitable() :mixed{
;

HH\Asio\join(foo());
}
