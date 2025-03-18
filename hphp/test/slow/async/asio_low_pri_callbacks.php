<?hh

<<__AsioLowPri>>
async function lowpri(): Awaitable<void> {
  await SleepWaitHandle::create(1000000); // 1 second
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  AsyncFunctionWaitHandle::setOnCreateCallback(($wh, $child) ==> {
    echo "onCreateCallback()...\n";
    var_dump($child);
  });

  await lowpri();
}
