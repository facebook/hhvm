<?hh
require(__DIR__ . '/common.inc');

$path = __DIR__ . "/breakpoints.php.test";
$GLOBALS["path"] = $path;
$GLOBALS["file"] = "breakpoints.php.test";

// Calibration mappings:
$mapping[9] = 9;
$mapping[10] = 10;
$mapping[12] = 13;
$mapping[19] = 20;
$mapping[26] = 23;
$mapping[29] = 29;
$GLOBALS['mapping'] = $mapping;

/*
 * Breakpoint tests - valid breakpoints, resolution, calibration
 */
$setBreakpointsCommand = array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 3,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "breakpoints.php.test"
      ),
    "lines" => [9, 10, 12],
    "breakpoints" => [
      array("line" => 9, "condition" => ""),
      array("line" => 10, "condition" => ""),
      array("line" => 12, "condition" => "")
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
      array("id" => 3, "verified" => false)
    )));

$testProcess = vsDebugLaunch(
  __DIR__ . '/breakpoints.php.test',
  true,
  [$setBreakpointsCommand],
  [
    // New BP event for each breakpoint
    bpEvent($path, 9, 1, "new", false),
    bpEvent($path, 10, 2, "new", false),
    bpEvent($path, 12, 3, "new", false),

    // Response event
    $setBreakpointsRepsponse,

    // Resolved BP event for each bp
    bpEvent($path, 9, 1, "changed", true),
    bpEvent($path, 10, 2, "changed", true),
    bpEvent($path, 12, 3, "changed", true)
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
resumeTarget();

// Verify we hit breakpoint 2.
verifyBpHit(2, 10);
resumeTarget();

// Verify we hit breakpoint 3.
verifyBpHit(3, 12);

// Set breakpoint after the end of the file.
sendVsCommand(array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 5,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "breakpoints.php.test"
      ),
    "lines" => [200],
    "breakpoints" => [
      array("line" => 200, "condition" => ""),
    ])));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, bpEvent($path, 200, 4, "new", false));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "setBreakpoints",
  "request_seq" => 5,
  "success" => true,
  "body" => array(
      "breakpoints" => [
        array("id" => 4, "verified" => false),
      ]
  )));

// Setting a breakpoint after the end of the file should generate a warning.
// Two warnings: one when the bp is set, one when the request tries to resolve
// it.
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array(
    "category" => "warning"
  )));

// Setting a breakpoint with an invalid line number should fail.
  sendVsCommand(array(
    "command" => "setBreakpoints",
    "type" => "request",
    "seq" => 6,
    "arguments" => array(
      "source" =>
        array(
          "path" => $path,
          "name" => "breakpoints.php.test"
        ),
      "lines" => [-1],
      "breakpoints" => [
        array("line" => -1, "condition" => ""),
      ])));

  // Failure response.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "setBreakpoints",
    "request_seq" => 6,
    "success" => false,
  ));

  // And an error should be sent for the user.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "output",
    "body" => array(
      "category" => "error"
    )));


// Test breakpoint calibration
$setBreakpointsCommand = array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 7,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "breakpoints.php.test"
      ),
    "lines" => [19, 26],
    "breakpoints" => [
      array("line" => 19, "condition" => ""),
      array("line" => 26, "condition" => "")
    ]
  ));
sendVsCommand($setBreakpointsCommand);

// Expect calibrated new bp events.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, bpEvent($path, 19, 5, "new", false));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, bpEvent($path, 26, 6, "new", false));

// Expect a set breakpoints response.
$setBreakpointsRepsponse = array(
  "type" => "response",
  "command" => "setBreakpoints",
  "success" => true,
  "request_seq" => 7,
  "body" => array(
    "breakpoints" => array(
      array("id" => 5, "verified" => false),
      array("id" => 6, "verified" => false)
    )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $setBreakpointsRepsponse);

// Expect calibrated breakpoint verified events.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, bpEvent($path, 19, 5, "changed", true));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, bpEvent($path, 26, 6, "changed", true));

// Remove all breakpoints.
sendVsCommand(array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 4,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "/breakpoints.php.test"
      ),
    "lines" => [],
    "breakpoints" => [])));

// Response should indicate no breakpoints remaining.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "setBreakpoints",
  "request_seq" => 4,
  "success" => true,
  "body" => array(
      "breakpoints" => []
  )));

// Verify hitting a bp inside a loop and then removing it doesn't hit it again.
sendVsCommand(array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 5,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "/breakpoints.php.test"
      ),
    "lines" => [29],
    "breakpoints" => [
      array("line" => 29, "condition" => ""),
    ])));

// New bp event.
$msg = json_decode(getNextVsDebugMessage(), true);
// Response.
$msg = json_decode(getNextVsDebugMessage(), true);
// Resolved event.
$msg = json_decode(getNextVsDebugMessage(), true);

resumeTarget();

// Bp hit.
verifyBpHit(7, 29);
resumeTarget();

// Bp hit again.
verifyBpHit(7, 29, 2);

// Remove the breakpoint and resume, it should not hit again now even
// though the loop has 8 more iterations.
sendVsCommand(array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 8,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "/breakpoints.php.test"
      ),
    "lines" => [],
    "breakpoints" => [])));
// Response.
$msg = json_decode(getNextVsDebugMessage(), true);
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
