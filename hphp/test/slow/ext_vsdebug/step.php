<?hh
require(__DIR__ . '/common.inc');

$path = __DIR__ . "/step.php.test";
$GLOBALS["path"] = $path;
$GLOBALS["file"] = "step.php.test";

// Calibration mappings:
$mapping[9] = 9;
$mapping[13] = 13;
$GLOBALS['mapping'] = $mapping;

function verifyStopLine($frames, $line) {
  global $path;

  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "stopped",
    "body" => array(
        "threadId" => 1
    )));

  sendVsCommand(array(
    "command" => "stackTrace",
    "type" => "request",
    "seq" => 1,
    "arguments" => array(
      "threadId" => 1
    )));

  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "stackTrace",
    "request_seq" => 1,
    "success" => true,
    "body" => array(
        "totalFrames" => $frames,
        "stackFrames" => [
          array(
            "source" => array("path" => $path, "name" => $path),
            "line" => $line,
          )
        ]
      )
    ));
}

function stepCommand($cmd) {
  sendVsCommand(array(
    "command" => $cmd,
    "type" => "request",
    "seq" => 5,
    "arguments" => array("threadId" => 1)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => $cmd,
    "request_seq" => 5,
    "success" => true));

  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "continued",
    "body" => array(
        "threadId" => 1
    )));
}

$setBreakpointsCommand = array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 3,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "step.php.test"
      ),
    "lines" => [9, 13],
    "breakpoints" => [
      array("line" => 9, "condition" => ""),
      array("line" => 13, "condition" => "")
    ]
  ));

$setBreakpointsRepsponse = array(
  "type" => "response",
  "command" => "setBreakpoints",
  "success" => true,
  "request_seq" => 3,
  "body" => array(
    "breakpoints" => array(
      array("id" => 1, "verified" => false),
      array("id" => 2, "verified" => false),
    )));

$testProcess = vsDebugLaunch(
  __DIR__ . '/step.php.test',
  true,
  [$setBreakpointsCommand],
  [
    // Response event
    $setBreakpointsRepsponse,

    // New BP event for each breakpoint
    bpEvent($path, 9, 1, "new", false),
    bpEvent($path, 13, 1, "new", false),

    // Resolved BP event for each bp
    bpEvent($path, 9, 1, "changed", true),
    bpEvent($path, 13, 1, "changed", true)
  ]
);

// Verify we resumed from loader break.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued",
  "body" => array(
      "allThreadsContinued" => false
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued",
  "body" => array(
      "allThreadsContinued" => true,
      "threadId" => 1
  )));

// Verify we hit breakpoint 1.
verifyBpHit(1, 9);

// Step over.
stepCommand("next");
verifyStopLine(1, 10);

// Step in
stepCommand("stepIn");
verifyStopLine(2, 4);

// Step over.
stepCommand("next");
verifyStopLine(2, 5);

// Step out.
stepCommand("stepOut");
verifyStopLine(1, 11);

// Step over function.
stepCommand("next");
verifyStopLine(1, 12);

// Continue to location hits bp on the way to the target line.
sendVsCommand(array(
  "command" => "fb_continueToLocation",
  "type" => "request",
  "seq" => 5,
  "arguments" =>
    array(
      "threadId" => 1,
      "source" => array("path" => $path),
      "line" => 15
    )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array(
      "category" => "info",
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "fb_continueToLocation",
  "request_seq" => 5));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued"));

verifyBpHit(2, 13);

// Resume and exit.
resumeTarget();

// Verify that the script exited.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "thread",
  "body" => array(
    "threadId" => 1,
    "reason" => "exited"
  )));

// Read anything left it stdout and stderr and echo it.
$stdout = $testProcess[1][1];
$stderr = $testProcess[1][2];
echo stream_get_contents($stdout);
echo stream_get_contents($stderr);
vsDebugCleanup($testProcess[0], $testProcess[1], $testProcess[2]);
