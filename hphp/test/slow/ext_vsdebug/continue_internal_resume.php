<?hh
<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
// Breakpoints inside foo() (line 4) and at first statement in main (line 9).
$breakpoints = vec[
   dict[
     "path" => __FILE__ . ".test",
     "breakpoints" => vec[
       dict["line" => 4, "calibratedLine" => 4, "condition" => ""],
       dict["line" => 9, "calibratedLine" => 9, "condition" => ""],
     ]]
   ];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);
// Skip breakpoint resolution messages.
skipMessages(count($breakpoints[0]['breakpoints']));

// Verify we hit breakpoint at line 9 (in main).
verifyBpHit($breakpoints[0]['path'], $breakpoints[0]['breakpoints'][1]);

// Query stack trace to populate frame objects (needed for evaluate routing).
$seq = sendVsCommand(dict[
  "command" => "stackTrace",
  "type" => "request",
  "arguments" => dict["threadId" => 1]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
    "totalFrames" => 1,
    "stackFrames" => vec[
      dict[
        "source" => dict["path" => __FILE__ . ".test"],
        "id" => 1,
        "line" => 9,
        "name" => "main"
      ]]]]);

// Evaluate foo(), which hits the breakpoint at line 4 during evaluation
// (nested pause, pauseRecurseCount > 1).
$evalSeq = sendVsCommand(dict[
  "command" => "evaluate",
  "type" => "request",
  "arguments" => dict[
    "expression" => 'foo();',
    "context" => "repl",
    "frameId" => 1,
  ]]);

// Breakpoint changed event (hit count updated for line 4 bp).
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "event",
  "event" => "breakpoint",
  "body" => dict[
    "reason" => "changed",
    "breakpoint" => dict[
      "originalLine" => 4,
      "verified" => true,
      "endLine" => 4,
  ]]]);

// Stopped event for the nested pause.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "event",
  "event" => "stopped",
  "body" => dict[
    "threadId" => 1,
  ]]);

// Send continue with threadId to trigger the pauseRecurseCount > 1 path.
// This dispatches an internal ContinueCommand via createInstance() (WorkItem)
// which must NOT produce a response.
$contSeq = sendVsCommand(dict[
  "command" => "continue",
  "type" => "request",
  "arguments" => dict["threadId" => 1]]);

// Continue response for the client-initiated continue.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "continue",
  "success" => true,
  "request_seq" => $contSeq]);

// After the internal resume: continued event, re-stopped event (back at outer
// breakpoint), and evaluate response. Verify no spurious continue response
// from the internal createInstance() command.
$gotContinuedEvent = false;
$gotReStoppedEvent = false;
$gotEvalResponse = false;

for ($i = 0; $i < 3; $i++) {
  $msg = json_decode(getNextVsDebugMessage(), true);

  if ($msg['type'] === 'event' && $msg['event'] === 'continued') {
    $gotContinuedEvent = true;
  } else if ($msg['type'] === 'event' && $msg['event'] === 'stopped') {
    $gotReStoppedEvent = true;
    checkObjEqualRecursively($msg, dict[
      "type" => "event",
      "event" => "stopped",
      "body" => dict["threadId" => 1]]);
  } else if ($msg['type'] === 'response' && $msg['command'] === 'evaluate') {
    $gotEvalResponse = true;
    checkObjEqualRecursively($msg, dict[
      "type" => "response",
      "command" => "evaluate",
      "success" => true,
      "request_seq" => $evalSeq]);
  } else if ($msg['type'] === 'response' && $msg['command'] === 'continue') {
    throw new UnexpectedValueException(
      "Spurious continue response from internal createInstance() resume. " .
      "Message: " . json_encode($msg));
  }
}

if (!$gotContinuedEvent) {
  throw new UnexpectedValueException("Missing continued event after inner resume");
}
if (!$gotReStoppedEvent) {
  throw new UnexpectedValueException("Missing re-stopped event at outer breakpoint");
}
if (!$gotEvalResponse) {
  throw new UnexpectedValueException("Missing evaluate response");
}

// Resume from the outer breakpoint.
resumeTarget();

checkForOutput($testProcess, "1", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
}
