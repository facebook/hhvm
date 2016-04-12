<?hh

function block() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
};

async function ret1() {
  await block();
  return 1;
}

async function await1() {
  await block();
  $b = await ret1();
  return 1 + $b;
}

var_dump(HH\Asio\join(ret1()));
var_dump(HH\Asio\join(await1()));
