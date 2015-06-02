<?hh

function block() { return RescheduleWaitHandle::create(1,1); };

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

var_dump(retValue());

