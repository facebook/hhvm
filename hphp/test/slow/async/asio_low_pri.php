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

  // Even though lowpri() is started eagerly, it will be suspended when
  // annotated with __AsioLowPri. Then, we expect normal() to print logs first
  // but finish last (due to a longer sleep).
  $low = lowpri();
  await normal();
  await $low;

  echo("end main()\n");
}
