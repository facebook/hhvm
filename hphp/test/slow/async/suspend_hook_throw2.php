<?hh

function thrower($why, $what) {
  if ($why == 'exit' && $what == 'foo') {
    throw new exception;
  }
}
fb_setprofile('thrower');

class dtor {
  function __construct(private $i) {}
  function __destruct() { echo "dtor: $this->i\n"; }
}

async function foo($resched) {
  $dtor = new dtor(1);
  echo "enter foo\n";
  await $resched;
  echo "foo fallthrhough\n";
}

async function thing() {
  echo "thing1\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "thing2\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "thing3\n";
}
$y = thing();

function get($y) {
  try {
    HH\Asio\join(foo($y));
  } catch (exception $ex) {
    echo "caught in get\n";
  }
  echo "leaving get\n";
}

$x = get($y);
// Dtor1 should print after this, because the wait handle is still alive and
// blocked on thing().
echo "done\n";
