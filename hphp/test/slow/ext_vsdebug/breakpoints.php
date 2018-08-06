<?hh
require(__DIR__ . '/common.inc');

$breakpoints = [
   array(
     "path" => __FILE__ . ".test",
     "breakpoints" => [
       array("line" => 9, "calibratedLine" => 9, "condition" => ""),
       array("line" => 10, "calibratedLine" => 10, "condition" => ""),
       array("line" => 12, "calibratedLine" => 16, "condition" => "",
        "multiLine" => true),
     ])
   ];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);

checkForOutput($testProcess, "Hello world.\n", "stdout");

// Verify we hit breakpoint 1.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);
resumeTarget();

// Verify we hit breakpoint 2.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[1]);
resumeTarget();

// Verify we hit breakpoint 3.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[2]);

// Set a breakpoint past the end of the file. This breakpoint will never verify.
$breakpoints = [
  array(
    "path" => __FILE__ . ".test",
    "breakpoints" => [
      array("line" => 200, "calibratedLine" => 200, "condition" => ""),
    ])
  ];
setBreakpoints($breakpoints, false);

// Test breakpoint calibration
$breakpoints = [
  array(
    "path" => __FILE__ . ".test",
    "breakpoints" => [
      array("line" => 19, "calibratedLine" => 20, "condition" => ""),
      array("line" => 22, "calibratedLine" => 26, "condition" => ""),
    ])
  ];
setBreakpoints($breakpoints, true);

// Setting a breakpoint with an invalid line number should fail.
$setBreakpointsCommand = array(
  "command" => "setBreakpoints",
  "type" => "request",
  "arguments" => array(
    "source" =>
      array(
        "path" => __FILE__ . ".test",
        "name" => "test"
      ),
    "breakpoints" => [
        array("line" => -1, "condition" => "")
    ]
  ));
$seq = sendVsCommand($setBreakpointsCommand);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg,
  array(
    "type" => "response",
    "command" => "setBreakpoints",
    "success" => false,
    "message" => "Breakpoint has invalid line number.",
    "request_seq" => $seq));

// An error is displayed to the user.
checkForOutput($testProcess, "Breakpoint has invalid line number.", "stderr",
              true);

// Remove all breakpoints.
$seq = sendVsCommand(array(
  "command" => "setBreakpoints",
  "type" => "request",
  "arguments" => array(
    "source" =>
      array(
        "path" => __FILE__ . ".test",
        "name" => "test"
      ),
    "lines" => [],
    "breakpoints" => [])));

// Response should indicate no breakpoints remaining.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "setBreakpoints",
  "request_seq" => $seq,
  "success" => true,
  "body" => array(
      "breakpoints" => []
  )));

$breakpoints = [
  array(
    "path" => __FILE__ . ".test",
    "breakpoints" => [
      array("line" => 29, "calibratedLine" => 29, "condition" => ""),
    ])
  ];
setBreakpoints($breakpoints, true);
resumeTarget();

checkForOutput($testProcess, "12", "stdout");
checkForOutput($testProcess, "\n", "stdout");

checkForOutput($testProcess, "12", "stdout");
checkForOutput($testProcess, "\n", "stdout");

// Hit the breakpoint inside the loop on line 29
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0], 1);
resumeTarget();

// Hit the breakpoint inside the loop again.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0], 2);

// Remove the breakpoint and resume, it should not hit again now even
// though the loop has 8 more iterations.
$seq = sendVsCommand(array(
  "command" => "setBreakpoints",
  "type" => "request",
  "arguments" => array(
    "source" =>
      array(
        "path" => __FILE__ . ".test",
        "name" => "test"
      ),
    "lines" => [],
    "breakpoints" => [])));

// Response should indicate no breakpoints remaining.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "setBreakpoints",
  "request_seq" => $seq,
  "success" => true,
  "body" => array(
      "breakpoints" => []
  )));
resumeTarget();

// Verify hard break was hit.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "stopped",
  "body" => array(
      "threadId" => 1,
      "reason" => "breakpoint",
      "description" => "hphp_debug_break()",
  )));

resumeTarget();

checkForOutput($testProcess, "goodbye.\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
