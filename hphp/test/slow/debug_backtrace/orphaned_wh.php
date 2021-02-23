<?hh

<<__EntryPoint>>
async function main(): Awaitable<void> {
  A();
  await RescheduleWaitHandle::create(0, 2);
  echo "complete\n";
}

async function A(): Awaitable<void> {
  $v = new Vector();
  $v[] = B($v);
  await RescheduleWaitHandle::create(0, 1);
  await $v[0];
}

async function B($v): Awaitable<void> {
  await C($v);
}

async function C($v): Awaitable<void> {
  await AwaitAllWaitHandle::fromVec(vec[$v[0]]);
}
