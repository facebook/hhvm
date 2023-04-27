<?hh

async function recurse(): Awaitable<void> {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  await recurse();
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  ini_set('memory_limit', '50M');
  await recurse();
}
