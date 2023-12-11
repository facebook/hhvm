<?hh

// Basic Xenon test.  PHP stacks but no Async stacks.

async function fa3($a) :Awaitable<mixed>{
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  $a = $a * 101;
  return $a;
}

function fn1($a) :mixed{
  return 19 + \HH\Asio\join(fa3($a +3));
}

async function fa2($a) :Awaitable<mixed>{
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  return 12 + fn1($a + 2);
}

async function fa1($a) :Awaitable<mixed>{
  $values = await \HH\Asio\v(vec[
    fa2($a),
  ]);
  return 3 * $values[0];
}

function fn0($a) :mixed{
  return 2 * \HH\Asio\join(fa1(1 + $a));
}

async function fa0($a) :Awaitable<mixed>{
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  return fn0($a);
}

function main($a) :mixed{
  return \HH\Asio\join(fa0($a));
}
<<__EntryPoint>>
function entrypoint_xenon_init(): void {

  include "xenonUtil.inc";

  echo main(42) . "\n";

  // get the Xenon data then verify that there are no unknown functions
  // and that all of the functions in this file are in the stack
  $stacks = xenon_get_data();
  $required_functions = vec[
    'HH\Asio\join',
    'HH\Asio\v',

    'fa0',
    'fa1',
    'fa2',
    'fa3',
    'fn0',
    'fn1',
    'main',
    'entrypoint_xenon_init',
  ];
  $optional_functions = vec[
    'HH\Asio\result',
    'include',
    'count',
    Vector::class.'::__construct',
    AwaitAllWaitHandle::class.'::fromVec',
    RescheduleWaitHandle::class.'::create',
    'HH\array_mark_legacy',
  ];

  verifyTestRun($stacks, $required_functions, $optional_functions);
}
