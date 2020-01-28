<?hh
require(__DIR__ . '/common.inc');
<<__EntryPoint>> function main(): void {
$breakpoints = varray[
   darray[
     "path" => __FILE__ . ".test",
     "breakpoints" => varray[
       darray["line" => 3, "calibratedLine" => 3, "condition" => ""],
     ]]
   ];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);

verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

$exnBpCommand = darray[
  "command" => "setExceptionBreakpoints",
  "type" => "request",
  "arguments" => darray[
    "exceptionOptions" => darray[
      "breakMode" => "always"
    ]
  ]];

$seq = sendVsCommand($exnBpCommand);
$exnBpResp = darray[
  "type" => "response",
  "command" => "setExceptionBreakpoints",
  "success" => true,
  "request_seq" => $seq,
];
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $exnBpResp);


resumeTarget();
checkForOutput($testProcess, "hello world.\n", "stdout");

// See the exception output.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, darray[
  "type" => "event",
  "event" => "output",
  "body" => darray[
      "category" => "console",
      "output" =>
        "Request 1: Exception (UnexpectedValueException) thrown: Exn thrown!"
  ]]);

// Verify we stopped on exception thrown.
$exnStopObj = darray[
  "type" => "event",
  "event" => "stopped",
  "body" => darray[
      "threadId" => 1,
      "reason" => "exception",
      "description" => "Exception (UnexpectedValueException) thrown"
  ]];
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $exnStopObj);

// Set exn breaks to an invalid value.
$exnBpCommand = darray[
  "command" => "setExceptionBreakpoints",
  "type" => "request",
  "arguments" => darray[
    "exceptionOptions" => darray[
      "breakMode" => "INVALID"
    ]
  ]];
sendVsCommand($exnBpCommand);

$exnBpResp = darray[
  "type" => "response",
  "command" => "setExceptionBreakpoints",
  "success" => false];
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $exnBpResp);

// Set to break on unhandled only - this currently generates a warning on
// exception but doesn't break.
$exnBpCommand = darray[
  "command" => "setExceptionBreakpoints",
  "type" => "request",
  "arguments" => darray[
    "exceptionOptions" => darray[
      "breakMode" => "unhandled"
    ]
  ]];
sendVsCommand($exnBpCommand);

$exnBpResp = darray[
  "type" => "response",
  "command" => "setExceptionBreakpoints",
  "success" => true];
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $exnBpResp);

resumeTarget();

// See a warning when the next exception is thrown, but no breakpoint.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, darray[
  "type" => "event",
  "event" => "output",
  "body" => darray[
    "category" => "console",
    "output" =>
      "Request 1: Exception (UnexpectedValueException) thrown: Exn thrown!",
  ]]);

checkForOutput($testProcess, "About to throw again.\n", "stdout");

// See a warning when the next exception is thrown, but no breakpoint.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, darray[
  "type" => "event",
  "event" => "output",
  "body" => darray[
    "category" => "console",
    "output" =>
      "Request 1: Exception (UnexpectedValueException) thrown: Exn thrown!",
  ]]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, darray[
  "type" => "event",
  "event" => "output",
  "body" => darray[
    "category" => "stderr",
  ]]);

$expected = "\nFatal error: Uncaught exception 'UnexpectedValueException' "
  . "with message 'Exn thrown!'";
if (substr($msg{'body'}{'output'}, 0, strlen($expected)) !== $expected) {
  throw new UnexpectedValueException("Unexpected warning message");
}

vsDebugCleanup($testProcess);

echo "OK!\n";
}
