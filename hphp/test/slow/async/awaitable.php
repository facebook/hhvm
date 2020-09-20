<?hh

function block() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function f() {
  return 1;
}

async function foo() {
  $f = f();
  $a = await $f;
  var_dump($a);
  await block();
  $f = f();
  $a = await $f;
  var_dump($a);
}

<<__EntryPoint>>
function main_awaitable() {
;

HH\Asio\join(foo());
}
