<?hh // decl

function boom() {
  echo "exiting\n";
  exit(1);
}

async function block() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function failme() { await block(); throw new Exception; }

async function crash() {
  await block();
  $block = block();
  echo "triggering handler\n";
  await failme();
  echo "should have exited!\n";
}

<<__EntryPoint>>
function main_await_cpp_exception() {
  ResumableWaitHandle::setOnFailCallback(($wh, $e) ==> boom());
  HH\Asio\join(crash());
}
