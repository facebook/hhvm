<?hh

function reschedule() :mixed{
  return RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

function pretty_backtrace() :mixed{
  $stack = debug_backtrace();
  array_shift(inout $stack);
  echo "backtrace:\n";
  foreach ($stack as $frame) {
    printf("  %s\n", $frame['function']);
    if ($frame['line'] ?? false) {
      printf("    called from line %d\n", $frame['line']);
    }
    if ($frame['function'] === 'main') break;
  }
}

async function suspends() :Awaitable<mixed>{
  await reschedule();
  pretty_backtrace();
  return 1;
}

async function eager() :Awaitable<mixed>{
  pretty_backtrace();
  return 2;
}

<<__EntryPoint>> async function main() :Awaitable<mixed>{

  /* put each token on its own line and add comments */
  concurrent
  /* because we're testing position */
  {
    $x = await suspends();
    $y = await eager();
    await suspends();
    await eager();
    $z = await suspends();
    $w = await eager();
  }

  var_dump($x, $y, $z, $w);

  /* we lift awaits from statements too */
  var_dump(
    /* so let's test this flavor */
    await suspends()
    /* as well */
    +
    /* testing */
    await eager()
  /* testing */
  )
  /* testing */
  ;
}
