<?hh

class E1 extends Exception {
  private static $e;

  function __construct($msg) {
    parent::__construct($msg);
    if (self::$e === null) {
      self::$e = $this;
    }
  }
  function __destruct() { var_dump(__METHOD__); }

  function rethrow() {
    $e = self::$e;
    self::$e = null;
    throw $e;
  }
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
  return AwaitAllWaitHandle::fromArray(array(fuz(), fuz()));
}

async function bar() {
  try {
    await baz();
    throw E1::rethrow();
  } catch (Exception $e) {
    var_dump($e->getMessage());
    return;
  }
}

function test() {
  \HH\Asio\join(bar());
}

test();
