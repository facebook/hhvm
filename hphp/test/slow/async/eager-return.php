<?hh

function reschedule($priority) :mixed{
  $queue = RescheduleWaitHandle::QUEUE_DEFAULT;
  return RescheduleWaitHandle::create($queue, $priority);
}

async function foo($x) :Awaitable<mixed>{
  if ($x == 1) await reschedule(0);
  return 42;
}

async function baz($x) :Awaitable<mixed>{
  return await foo($x);
}

async function bar($x) :Awaitable<mixed>{
  if ($x & 4) {
    $x &= 3;
    await reschedule(0);
  }
  if ($x == 2) return await baz(1);
  return await foo($x);
}

async function run() :Awaitable<mixed>{
  for ($i =0; $i < 7; $i++) {
    try {
      $res = await bar($i);
      var_dump($res);
    } catch (Exception $e) {
      var_dump("Caught exception: i=$i");
    }
  }
}


<<__EntryPoint>>
async function main_eager_return() :Awaitable<mixed>{
  await run();
  fb_setprofile(function($when, $func, $args) {
    if ($when == 'exit' &&
        $func == 'baz' &&
        $args === null) {
      //var_dump($when, $func, $args);
      throw new Exception;
    }
  });
  await run();
}
