<?hh

async function genWasteMilliseconds($ms) {
  await SleepWaitHandle::create($ms * 1000);
}

function randomFunction() {
  rand() * rand();
}

async function genLongFunction() {
  $test_time_seconds = 5;
  set_time_limit(10);
  $start = time();
  $vecI = Vector { 0 };
  set_pre_timeout_handler(1, () ==> {
    $now = time() - $start;
    randomFunction();
    echo "$now seconds into the request on callback1 (i={$vecI[0]})\n";
    set_pre_timeout_handler(1, () ==> {
      $now = time() - $start;
      randomFunction();
      echo "$now seconds into the request on callback2 (i={$vecI[0]})\n";
      set_pre_timeout_handler(-5, () ==> {
        echo "This should never happen\n";
      });
    });
  });

  // Sleep X amount of seconds in intervals of 100ms
  for ($i = 0; $i < ($test_time_seconds * 10); ++$i) {
    $vecI[0] = $i;
    await genWasteMilliseconds(100);
  }
}

<<__EntryPoint>>
function main() {
  \HH\Asio\join(genLongFunction());
}
