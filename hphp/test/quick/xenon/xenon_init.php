<?hh

include "xenonUtil.inc";

// Basic Xenon test.  PHP stacks but no Async stacks.

async function fa3($a) {
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  $a = $a * 101;
  return $a;
}

function fn1($a) {
  return 19 + \HH\Asio\join(fa3($a +3));
}

async function fa2($a) {
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  return 12 + fn1($a + 2);
}

async function fa1($a) {
  $values = await \HH\Asio\v(array(
    fa2($a),
  ));
  return 3 * $values[0];
}

function fn0($a) {
  return 2 * \HH\Asio\join(fa1(1 + $a));
}

async function fa0($a) {
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  return fn0($a);
}

function main($a) {
  return \HH\Asio\join(fa0($a));
}

echo main(42) . "\n";

// get the Xenon data then verify that there are no unknown functions
// and that all of the functions in this file are in the stack
$stacks = xenon_get_data();
$required_functions = array(
  'include',
  'HH\Asio\join',
  'HH\Asio\v',
  'HH\Asio\result',

  'fa0',
  'fa1',
  'fa2',
  'fa3',
  'fn0',
  'fn1',
  'main',
);
$optional_functions = array(
  'count',
  Vector::class.'::reserve',
  AwaitAllWaitHandle::class.'::fromVector',
  RescheduleWaitHandle::class.'::create',
  WaitHandle::class.'::getWaitHandle',
);

verifyTestRun($stacks, $required_functions, $optional_functions);
