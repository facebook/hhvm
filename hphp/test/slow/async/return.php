<?hh

function block() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function f1() { return; }
async function f2() {  }
async function f3() { await f1(); }
async function f4() { return null; }

async function b1() { await block(); return; }
async function b2() { await block(); }
async function b3() { await block(); await f1(); }
async function b4() { await block(); return null; }

<<__EntryPoint>>
function main_return() {
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
