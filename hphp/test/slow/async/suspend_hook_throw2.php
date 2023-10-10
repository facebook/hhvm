<?hh

function thrower($why, $what) :mixed{
  if ($why == 'exit' && $what == 'foo') {
    throw new Exception;
  }
}

async function foo($resched) :Awaitable<mixed>{
  echo "enter foo\n";
  await $resched;
  echo "foo fallthrhough\n";
}

async function thing() :Awaitable<mixed>{
  echo "thing1\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "thing2\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "thing3\n";
}

function get($y) :mixed{
  try {
    HH\Asio\join(foo($y));
  } catch (Exception $ex) {
    echo "caught in get\n";
  }
  echo "leaving get\n";
}

<<__EntryPoint>>
function main_suspend_hook_throw2() :mixed{
fb_setprofile(thrower<>);
$y = thing();

$x = get($y);
// Dtor1 should print after this, because the wait handle is still alive and
// blocked on thing().
echo "done\n";
}
