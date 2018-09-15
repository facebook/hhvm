<?hh

function onCreateCallback() { echo "onCreateCallback()...\n"; }

async function answer() {
  await reschedule();
  return 42;
}function reschedule() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}

async function test() {
  list($a, $b, $c) = await genva(answer(), answer(), answer());
  return $a;
}

<<__EntryPoint>>
function main_await_all_callback() {
AwaitAllWaitHandle::setOnCreateCallback(
  ($a, $b) ==> onCreateCallback()
);
;
HH\Asio\join(test());
}
