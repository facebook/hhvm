<?hh

include "xenonUtil.inc";

// Test showing async stacks in Xenon.

async function genList(...$args) {
  await AwaitAllWaitHandle::fromArray($args);
  return array_map($wh ==> \HH\Asio\result($wh), $args);
}

async function gen1($a) {
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  return $a + 1;
}

async function gen2($a) {
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  $x = HH\Asio\join(gen1($a));
  return $x;
}

async function genBar($a) {
  await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
  return $a + 2;
}

async function genFoo($a) {
  $a++;
  list($x, $y) = await genList(
    genBar($a),
    genBar($a + 1),
    gen2($a + 2),
  );
  return $x + $y;
}

function idx($arr, $idx, $def = null) {
  return isset($arr[$idx]) ? $arr[$idx] : $def;
}

function main($a) {
  \HH\Asio\join(genFoo($a));
}

main(42);

// get the Xenon data then verify that there are no unknown functions
// and that all of the functions in this file are in the stack
$stacks = xenon_get_data();
$required_functions = array(
  'array_map',
  'include',
  'HH\Asio\join',
  'HH\Asio\result',

  'genList',
  'Closure$genList',
  'gen1',
  'gen2',
  'genFoo',
  'genBar',
  'main',
);
$optional_functions = array(
  AwaitAllWaitHandle::class.'::fromArray',
  RescheduleWaitHandle::class.'::create',
  WaitHandle::class.'::result',
);

verifyTestRun($stacks, $required_functions, $optional_functions);
