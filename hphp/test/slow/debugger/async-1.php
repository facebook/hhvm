<?hh

async function foo_async(): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  return 1;
}

async function bar_async(): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  return 2;
}

async function exn1_async(): Awaitable<int> {
  throw new Exception("exn");
}

async function exn2_async(): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  throw new Exception("exn");
}

<<__EntryPoint>>
function main() :mixed{
  echo __FILE__." loaded\n";
}
