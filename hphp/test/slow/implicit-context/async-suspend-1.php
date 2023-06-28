<?hh

async function answer() :Awaitable<mixed>{
  await reschedule();
  echo "In answer\n";
  return IntContext::getContext();
}

function reschedule() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}

async function change() :Awaitable<mixed>{
  echo "In change\n";
  return await IntContext::genStart(2, async () ==> {
    return IntContext::getContext();
  });
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  $result = await IntContext::genStart(1, async () ==> {
    concurrent {
      $x = await answer();
      $y = await change();
    }
    return $x + $y;
  });
  var_dump($result);
}
