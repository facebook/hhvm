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
  return dict[0 => 1];
}

async function consumes() :Awaitable<mixed>{
  $d = await suspends();
  return idx(
    $d,
    1,
    "default"
  );
}

<<__EntryPoint>> async function main() :Awaitable<mixed>{
  var_dump(await consumes());
}
