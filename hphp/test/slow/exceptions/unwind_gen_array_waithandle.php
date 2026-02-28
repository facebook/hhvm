<?hh

class E1 extends Exception {
  private static $e;

  function __construct($msg) {
    parent::__construct($msg);
    if (self::$e === null) {
      self::$e = $this;
    }
  }

  static function rethrow() :mixed{
    $e = self::$e;
    self::$e = null;
    throw $e;
  }
}

function reschedule($priority) :mixed{
  $queue = RescheduleWaitHandle::QUEUE_DEFAULT;
  return RescheduleWaitHandle::create($queue, $priority);
}

async function fuz() :Awaitable<mixed>{
  await reschedule(0);
  throw new E1("Boom");
}

function baz() :mixed{
  return AwaitAllWaitHandle::fromVec(vec[fuz(), fuz()]);
}

async function bar() :Awaitable<mixed>{
  try {
    await baz();
    throw E1::rethrow();
  } catch (Exception $e) {
    var_dump($e->getMessage());
    return;
  }
}

function test() :mixed{
  \HH\Asio\join(bar());
}


<<__EntryPoint>>
function main_unwind_gen_array_waithandle() :mixed{
test();
}
