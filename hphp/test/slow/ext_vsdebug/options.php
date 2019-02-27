<?hh
require(__DIR__ . '/common.inc');

function skipEvents() {
  $msg = json_decode(getNextVsDebugMessage(), true);
  while ($msg['type'] == 'event') {
    $msg = json_decode(getNextVsDebugMessage(), true);
  }

  return $msg;
}

$path = __FILE__ . ".test";
$testProcess = vsDebugLaunch($path, true, []);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "stopped",
  "body" => array(
      "threadId" => 1,
      "reason" => "breakpoint",
      "description" => "hphp_debug_break()",
  )));

$seq = sendVsCommand(array(
  "command" => "stackTrace",
  "type" => "request",
  "arguments" => array(
    "threadId" => 1
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => $seq,
  "success" => true,
  "body" => array("totalFrames" => 2)));

// Get bogus option throws
$seq = sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "arguments" =>
  array("frameId" => 1,
    "expression" => "hphp_debugger_get_option('BOGUS OPTION')")));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => [
    "output" => "Hit fatal : setDebuggerOption: Unknown option specified\n"
  ]));

// This raised a notice, which will spew a bunch of error to stdout.
// Skip past that.
$msg = skipEvents();

// Response to the failed eval.
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => $seq,
  "success" => false));

// Get valid option succeeds.
$seq = sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "arguments" =>
  array("frameId" => 1,
    "expression" => "hphp_debugger_get_option('showDummyOnAsyncPause')")));

$msg = skipEvents();

checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => $seq,
  "success" => true,
  "body" => [
    "result" => "false",
    "type" => "bool"
  ]));


// Set valid option succeeds.
$seq = sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "arguments" =>
  array("frameId" => 1,
    "expression" =>
      "hphp_debugger_set_option('showDummyOnAsyncPause', true)")));

$msg = skipEvents();

checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => $seq,
  "success" => true));

// Get valid option succeeds, sees the new value, and is case-insensitive
$seq = sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "arguments" =>
  array("frameId" => 1,
    "expression" => "hphp_debugger_get_option('showdummyonasyncPause')")));

$msg = skipEvents();

checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => $seq,
  "success" => true,
  "body" => [
    "result" => "true",
    "type" => "bool"
  ]));

echo "OK!\n";
