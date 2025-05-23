<?hh

<<__AsioLowPri>>
async function lowpri(): Awaitable<bool> {
  echo("start lowpri\n");
  await SleepWaitHandle::create(2000000); // 2 second
  echo("end lowpri\n");
  return true;
}

<<__AsioLowPri>>
async function lowpri_noop(): Awaitable<bool> {
  return true;
}

async function normal(Awaitable<bool> $low): Awaitable<bool> {
  echo("start normal\n");
  $bridge = PriorityBridgeWaitHandle::create($low);
  $bridge->prioritize();
  await SleepWaitHandle::create(1000000); // 1 second
  $res = await $bridge;
  echo("end normal\n");
  return $res;
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  $low = lowpri();
  $normal = normal($low);
  $res = HH\Asio\join($normal);
  var_dump($res);

  // Test prioritize() on a finished wait handle
  $bridge = PriorityBridgeWaitHandle::create(lowpri_noop());
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  $bridge->prioritize();
  $res = await $bridge;
  var_dump($res);
}
