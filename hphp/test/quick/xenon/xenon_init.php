<?hh

include "xenonUtil.inc";

// Basic Xenon test.  PHP stacks but no Async stacks.

async function fa3($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  $a = $a * 101;
  return $a;
}

function fn1($a) {
  return 19 + fa3($a +3)->join();
}

async function fa2($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  return 12 + fn1($a + 2);
}

async function fa1($a) {
  $values = await GenArrayWaitHandle::Create(
    array(
      fa2($a)
    )
  );
  return 3 * $values[0];
}

function fn0($a) {
  return 2 * fa1(1 + $a)->join();
}

function idx($arr, $idx, $def = null) {
  return isset($arr[$idx]) ? $arr[$idx] : $def;
}

async function fa0($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  return fn0($a);
}

function main($a) {
  return fa0($a)->join();
}

echo main(42) . "\n";

// get the Xenon data then verify that there are no unknown functions
// and that all of the functions in this file are in the stack
$stacks = xenon_get_data();
$functionList = array("fn1", "fn0", "main", "", "fa0", "fa1", "fa2", "fa3",
  "WaitHandle::join", "strcasecmp", "array_shift", "include");
$requiredFunctions = array(
  "fn1" => 1,
  "fn0" => 1,
  "main" =>1);

$asyncList = array("fa3", "fa2", "fa1", "fa0", "",
  "<gen-array>", "<prep>");
$requiredAsync = array("fa3" => 1, "fa2" => 1, "fa1" => 1, "fa0" => 1);

verifyTestRun($stacks, $functionList, $requiredFunctions,
  $asyncList, $requiredAsync);
