<?hh

function reschedule($priority) {
  $queue = RescheduleWaitHandle::QUEUE_DEFAULT;
  return RescheduleWaitHandle::create($queue, $priority);
}

async function foo($x) {
  if ($x == 1) await reschedule(0);
  return 42;
}

async function baz($x) {
  return await foo($x);
}

async function bar($x) {
  if ($x & 4) {
    $x &= 3;
    await reschedule(0);
  }
  if ($x == 2) return await baz(1);
  return await foo($x);
}

function main() {
  for ($i =0; $i < 7; $i++) {
    try {
      var_dump(bar($i)->join());
    } catch (Exception $e) {
      var_dump("Caught exception: i=$i");
    }
  }
}

main();
fb_setprofile(function($when, $func, $args) {
    if ($when == 'exit' &&
        $func == 'baz' &&
        $args === null) {
      #var_dump($when, $func, $args);
      throw new Exception;
    }
  });

main();
