<?hh

function onCreateCallback() { echo "onCreateCallback()...\n"; }
AwaitAllWaitHandle::setOnCreateCallback(
  ($a, $b) ==> onCreateCallback()
);

async function answer() {
  await reschedule();
  return 42;
};
function reschedule() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}

async function test() {
  list($a, $b, $c) = await genva(answer(), answer(), answer());
  return $a;
}
HH\Asio\join(test());
