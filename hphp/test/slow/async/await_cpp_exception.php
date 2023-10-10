<?hh

function boom() :mixed{
  echo "exiting\n";
  exit(1);
}

async function block() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function failme() :Awaitable<mixed>{ await block(); throw new Exception; }

async function crash() :Awaitable<mixed>{
  await block();
  $block = block();
  echo "triggering handler\n";
  await failme();
  echo "should have exited!\n";
}

<<__EntryPoint>>
function main_await_cpp_exception() :mixed{
  ResumableWaitHandle::setOnFailCallback(($wh, $e) ==> boom());
  HH\Asio\join(crash());
}
