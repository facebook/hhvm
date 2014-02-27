<?hh

function sleeps() {
  return SleepWaitHandle::create(2000000);
}

async function ret1() {
  $t1 = time();
  await sleeps();
  $t2 = time();

  if ($t2 - $t1 >= 2) {
    return 1;
  } else {
    return -1;
  }
}

async function await1() {
  $t1 = time();
  await sleeps();
  $ret = await ret1();
  $t2 = time();

  if ($t2 - $t1 >= 4) {
    return $ret + 1;
  } else {
    return -1;
  }
}

var_dump(ret1()->join());
var_dump(await1()->join());
