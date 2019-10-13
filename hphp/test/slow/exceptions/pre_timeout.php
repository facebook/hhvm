<?hh

async function genWasteMilliseconds($ms) {
  await SleepWaitHandle::create($ms * 1000);
}

async function genLongFunction() {
  $seconds = 4;
  $buffer = 20;  // asan can be really slow
  set_time_limit($seconds + $buffer);
  $start = time();
  $vecI = Vector { 0 };
  set_pre_timeout_handler(2 + $buffer, () ==> {
    $sec_remaining1 = time() - $start;
    echo "$sec_remaining1 into the request on callback1 (i={$vecI[0]})\n";
    set_pre_timeout_handler(1 + $buffer, () ==> {
      $sec_remaining2 = time() - $start;
      echo "$sec_remaining2 into the request on callback2 (i={$vecI[0]})\n";
    });
  });

  // Sleep X amount of seconds in intervals of 100ms
  for ($i = 0; $i < ($seconds * 10); ++$i) {
    $vecI[0] = $i;
    await genWasteMilliseconds(100);
  }
}

<<__EntryPoint>>
function main() {
  \HH\Asio\join(genLongFunction());
}
