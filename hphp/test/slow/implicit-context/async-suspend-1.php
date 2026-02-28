<?hh

async function answer() :Awaitable<mixed>{
  await reschedule();
  echo "In answer\n";
  return IntContext::getContext()->getPayload();
}

function reschedule() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}

async function change() :Awaitable<mixed>{
  echo "In change\n";
  return await IntContext::genStart(new Base(2), async () ==> {
    return IntContext::getContext()->getPayload();
  });
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  $result = await IntContext::genStart(new Base(1), async () ==> {
    concurrent {
      $x = await answer();
      $y = await change();
    }
    return $x + $y;
  });
  var_dump($result);
}
