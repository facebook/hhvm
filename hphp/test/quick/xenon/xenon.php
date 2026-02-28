<?hh

// Test showing async stacks in Xenon.

async function genList(...$args) :Awaitable<mixed>{
  await AwaitAllWaitHandle::fromVec(vec($args));
  return array_map($wh ==> \HH\Asio\result($wh), $args);
}

async function gen1($a) :Awaitable<mixed>{
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  return $a + 1;
}

async function gen2($a) :Awaitable<mixed>{
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  $x = HH\Asio\join(gen1($a));
  return $x;
}

async function genBar($a) :Awaitable<mixed>{
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  return $a + 2;
}

async function genFoo($a) :Awaitable<mixed>{
  $a++;
  list($x, $y) = await genList(
    genBar($a),
    genBar($a + 1),
    gen2($a + 2),
  );
  return $x + $y;
}

function idx($arr, $idx, $def = null) :mixed{
  return isset($arr[$idx]) ? $arr[$idx] : $def;
}

function main($a) :mixed{
  \HH\Asio\join(genFoo($a));
}
<<__EntryPoint>>
function entrypoint_xenon(): void {

  include "xenonUtil.inc";

  main(42);

  // get the Xenon data then verify that there are no unknown functions
  // and that all of the functions in this file are in the stack
  $success = false;
  $request_num = apc_fetch('request_number', inout $success);
  if ($success) {
    $stacks = HH\xenon_get_and_clear_samples();
  } else {
    $stacks = xenon_get_data();
  }
  $required_functions = vec[
    'array_map',
    'HH\Asio\join',
    'HH\Asio\result',

    'genList',
    'Closure$genList',
    'gen1',
    'gen2',
    'genFoo',
    'genBar',
    'main',
    'apc_fetch',
    'entrypoint_xenon',
  ];
  $optional_functions = vec[
    'include',
    'is_callable',
    AwaitAllWaitHandle::class.'::fromVec',
    RescheduleWaitHandle::class.'::create',
    Awaitable::class.'::result',
    'HH\array_mark_legacy',
  ];
  verifyTestRun($stacks, $required_functions, $optional_functions);
  if ($success) {
    apc_store('request_number', 2);
    $missed_sample_count = HH\xenon_get_and_clear_missed_sample_count();
    $y = HH\xenon_get_and_clear_missed_sample_count();
    if ($y !== 0) {
      echo "HH\xenon_get_and_clear_missed_sample_count() didn't reset the \
              counter\n";
    }
    if ($request_num === 2 && $missed_sample_count !== 2) {
      echo "stack traces thrown away is expected to be zero \
              for third request $missed_sample_count\n";
    }
    if ($request_num === 1 && $missed_sample_count < 3) {
      echo "stack traces thrown away($missed_sample_count) is \
            expected to be > 2 for second request\n";
    }
    HH\xenon_get_and_clear_samples();
  } else {
    apc_store('request_number', 1);
    $missed_sample_count = HH\xenon_get_and_clear_missed_sample_count();
    if ($missed_sample_count !== 0) {
      echo "stack traces thrown away is expected to be zero for first \
              request\n";
    }
  }
}
