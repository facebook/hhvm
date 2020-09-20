<?hh

function block() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function ret1() {
  await block();
  return 1;
}

async function await1() {
  $b = await ret1();
  return 1 + $b;
}

function retWh() { return await1(); }
function retValue() { return HH\Asio\join(retWh()); }


<<__EntryPoint>>
function main_pass_wh() {
;

var_dump(retValue());
}
