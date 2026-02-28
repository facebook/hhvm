<?hh

function block() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function ret1() :Awaitable<mixed>{
  await block();
  return 1;
}

async function await1() :Awaitable<mixed>{
  await block();
  $b = await ret1();
  return 1 + $b;
}

<<__EntryPoint>>
function main_simple_blocked() :mixed{
;

var_dump(HH\Asio\join(ret1()));
var_dump(HH\Asio\join(await1()));
}
