<?hh

include "xenonUtil.inc";

// Test showing async stacks in Xenon.

async function gen1($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  return $a + 1;
}

async function gen2($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  $x = HH\Asio\join(gen1($a));
  return $x;
}

async function genBar($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  return $a + 2;
}

async function genFoo($a) {
  $a++;
  list($x, $y) = await GenArrayWaitHandle::Create(
    array(
      genBar($a),
      genBar($a + 1),
      gen2($a + 2)
    )
  );
  return $x + $y;
}

function idx($arr, $idx, $def = null) {
  return isset($arr[$idx]) ? $arr[$idx] : $def;
}

function main($a) {
  $result = HH\Asio\join(genFoo($a));
}

main(42);

// get the Xenon data then verify that there are no unknown functions
// and that all of the functions in this file are in the stack
$stacks = xenon_get_data();
$functionList = array(
  'main',
  '',
  'HH\Asio\join',
  WaitHandle::class.'::join',
  'strcasecmp',
  'genFoo',
  'genBar',
  'gen1',
  'gen2',
  'array_shift',
  'include',
);
$requiredFunctions = array("main" => 1);

$asyncList = array("gen1", "gen2", "genBar", "genFoo", "",
  "<gen-array>", "<prep>");
$requiredAsync = array(
  "gen1" => 1,
  "gen2" => 1,
  "genBar" => 1,
  "genFoo" => 1
);

verifyTestRun($stacks, $functionList, $requiredFunctions,
  $asyncList, $requiredAsync);
