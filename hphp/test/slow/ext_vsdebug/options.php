<?hh

function skipEvents() :mixed{
  $msg = json_decode(getNextVsDebugMessage(), true);
  while ($msg['type'] == 'event') {
    $msg = json_decode(getNextVsDebugMessage(), true);
  }

  return $msg;
}
<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
$path = __FILE__ . ".test";
$testProcess = vsDebugLaunch($path, true, vec[]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "event",
  "event" => "stopped",
  "body" => dict[
      "threadId" => 1,
      "reason" => "breakpoint",
      "description" => "hphp_debug_break()",
  ]]);

$seq = sendVsCommand(dict[
  "command" => "stackTrace",
  "type" => "request",
  "arguments" => dict[
    "threadId" => 1
  ]]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict["totalFrames" => 2]]);

// Get bogus option throws
$seq = sendVsCommand(dict[
  "command" => "evaluate",
  "type" => "request",
  "arguments" =>
  dict["frameId" => 1,
    "expression" => "hphp_debugger_get_option('BOGUS OPTION')"]]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "event",
  "event" => "output",
  "body" => dict[
    "output" => "Hit fatal : getDebuggerOption: Unknown option specified\n"
  ]]);

// This raised a notice, which will spew a bunch of error to stdout.
// Skip past that.
$msg = skipEvents();

// Response to the failed eval.
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => $seq,
  "success" => false]);

// Get valid option succeeds.
$seq = sendVsCommand(dict[
  "command" => "evaluate",
  "type" => "request",
  "arguments" =>
  dict["frameId" => 1,
    "expression" => "hphp_debugger_get_option('showDummyOnAsyncPause')"]]);

$msg = skipEvents();

checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
    "result" => "false",
    "type" => "bool"
  ]]);


// Set valid option succeeds.
$seq = sendVsCommand(dict[
  "command" => "evaluate",
  "type" => "request",
  "arguments" =>
  dict["frameId" => 1,
    "expression" =>
      "hphp_debugger_set_option('showDummyOnAsyncPause', true)"]]);

$msg = skipEvents();

checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => $seq,
  "success" => true]);

// Get valid option succeeds, sees the new value, and is case-insensitive
$seq = sendVsCommand(dict[
  "command" => "evaluate",
  "type" => "request",
  "arguments" =>
  dict["frameId" => 1,
    "expression" => "hphp_debugger_get_option('showdummyonasyncPause')"]]);

$msg = skipEvents();

checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
    "result" => "true",
    "type" => "bool"
  ]]);

echo "OK!\n";
}
