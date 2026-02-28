<?hh

async function answer(): Awaitable<mixed>{
  await reschedule();
  echo "In answer\n";
  return TestAsyncContext::getContext();
}

function reschedule(): mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}

async function change(): Awaitable<mixed>{
  echo "In change\n";
  return await TestAsyncContext::genRunWith(2, async () ==> {
    return TestAsyncContext::getContext();
  });
}

<<__EntryPoint>>
async function main(): Awaitable<mixed>{
  include 'memo-agnostic-async.inc';

  $result = await TestAsyncContext::genRunWith(1, async () ==> {
    concurrent {
      $x = await answer();
      $y = await change();
    }
    return $x + $y;
  });
  var_dump($result);
}
