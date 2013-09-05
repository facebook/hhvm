<?hh

function block() { return RescheduleWaitHandle::create(1,1); };

async function ret1() {
  await block();
  return 1;
}

async function await1() {
  await block();
  $b = await ret1();
  return 1 + $b;
}

var_dump(ret1()->join());
var_dump(await1()->join());
