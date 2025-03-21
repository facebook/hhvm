<?hh

async function bar(): Awaitable<void> {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  echo(HH\asio_get_current_context_depth()."\n"); // 2
}

async function foo(): Awaitable<void> {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  echo(HH\asio_get_current_context_depth()."\n"); // 1
  HH\Asio\join(bar());
  echo(HH\asio_get_current_context_depth()."\n"); // 1
}

<<__EntryPoint>>
function main(): void {
  echo(HH\asio_get_current_context_depth()."\n"); // 0
  HH\Asio\join(foo());
  echo(HH\asio_get_current_context_depth()."\n"); // 0
}
