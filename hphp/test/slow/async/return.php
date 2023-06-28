<?hh

function block() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function f1() :Awaitable<mixed>{ return; }
async function f2() :Awaitable<mixed>{  }
async function f3() :Awaitable<mixed>{ await f1(); }
async function f4() :Awaitable<mixed>{ return null; }

async function b1() :Awaitable<mixed>{ await block(); return; }
async function b2() :Awaitable<mixed>{ await block(); }
async function b3() :Awaitable<mixed>{ await block(); await f1(); }
async function b4() :Awaitable<mixed>{ await block(); return null; }

<<__EntryPoint>>
function main_return() :mixed{
;
var_dump(HH\Asio\join(f1()));
var_dump(HH\Asio\join(f2()));
var_dump(HH\Asio\join(f3()));
var_dump(HH\Asio\join(f4()));
var_dump(HH\Asio\join(b1()));
var_dump(HH\Asio\join(b2()));
var_dump(HH\Asio\join(b3()));
var_dump(HH\Asio\join(b4()));
}
