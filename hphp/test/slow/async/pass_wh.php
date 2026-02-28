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
  $b = await ret1();
  return 1 + $b;
}

function retWh() :mixed{ return await1(); }
function retValue() :mixed{ return HH\Asio\join(retWh()); }


<<__EntryPoint>>
function main_pass_wh() :mixed{
;

var_dump(retValue());
}
