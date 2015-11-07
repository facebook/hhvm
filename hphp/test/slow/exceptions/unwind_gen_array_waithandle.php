<?hh

class E1 extends Exception {
  function __destruct() { var_dump(__METHOD__); }
}

function reschedule($priority) {
  $queue = RescheduleWaitHandle::QUEUE_DEFAULT;
  return RescheduleWaitHandle::create($queue, $priority);
}

async function fuz() {
  await reschedule(0);
  throw new E1("Boom");
}

function baz() {
  $a = array();
  $a[] = fuz()->getWaitHandle();
  $a[] = fuz()->getWaitHandle();
  return GenArrayWaitHandle::create($a);
}

async function bar() {
  try {
    await baz();
  } catch (Exception $e) {
    var_dump($e->getMessage());
    return;
  }
}

function test() {
  bar()->join();
}

test();
