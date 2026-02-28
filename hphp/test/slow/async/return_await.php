<?hh

function block() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function f1($a) :Awaitable<mixed>{ return "f1".$a; }
async function f2($a) :Awaitable<mixed>{ return await f1("f2".$a); }

async function f3($a) :Awaitable<mixed>{
  await block();
  return await f2("f3, ".$a);
}

<<__EntryPoint>>
function main_return_await() :mixed{
;

var_dump(HH\Asio\join(f2(".")));
var_dump(HH\Asio\join(f3("!")));
}
