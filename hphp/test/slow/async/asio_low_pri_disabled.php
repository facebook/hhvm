<?hh

<<__AsioLowPri>>
async function lowpri(): Awaitable<void> {
  echo("start lowpri\n");
  await SleepWaitHandle::create(1000000); // 1 second
  echo("end lowpri\n");
}

async function normal(): Awaitable<void> {
  echo("start normal\n");
  await SleepWaitHandle::create(2000000); // 2 second
  echo("end normal\n");
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  echo("start main()\n");

  // See await_low_pri.php for expected behavior when low pri awaitables are
  // enabled. When disabled we expect AwaitLowPri to noop, so output is eager
  // into lowpri().
  $low = lowpri();
  await normal();
  await $low;

  echo("end main()\n");
}
