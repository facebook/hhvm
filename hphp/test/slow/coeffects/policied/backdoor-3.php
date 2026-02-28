<?hh

async function f(): Awaitable<int> {
  echo "first\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "third\n";
  return 42;
}

<<__EntryPoint>>
async function main()[] :Awaitable<mixed>{
  $result = HH\Coeffects\backdoor(f<>);
  echo "second\n";
  echo (await $result)."\n";
  echo "fourth\n";
}
