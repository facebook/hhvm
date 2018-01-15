<?hh
require(__DIR__ . '/common.inc');

$path = __DIR__ . "/conditional_bp.php.test";
$GLOBALS["path"] = $path;
$GLOBALS["file"] = "conditional_bp.php.test";

// Breakpoint calibration mapping.
$mapping[4] = 4;
$mapping[5] = 5;
$mapping[6] = 6;
$mapping[7] = 7;

$GLOBALS['mapping'] = $mapping;

$setBreakpointsCommand = array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 3,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "conditional_bp.php.test"
      ),
    "lines" => [4, 5],
    "breakpoints" => [
      array("line" => 4, "condition" => "\$a == 2"),
      array("line" => 5, "condition" => "\$a == 1"),
      array("line" => 6, "condition" => "436fsl2$#^%l324;;'"),
      array("line" => 7, "condition" => "\"STRING\"")
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
      array("id" => 3, "verified" => false),
      array("id" => 4, "verified" => false)
    )));

$testProcess = vsDebugLaunch(
  __DIR__ . '/conditional_bp.php.test',
  true,
  [$setBreakpointsCommand],
  [
    // Response event
    $setBreakpointsRepsponse,

    // New BP event for each breakpoint
    bpEvent($path, 4, 1, "new", false),
    bpEvent($path, 5, 2, "new", false),
    bpEvent($path, 6, 3, "new", false),
    bpEvent($path, 7, 4, "new", false),

    // Resolved BP event for each bp
    bpEvent($path, 4, 1, "changed", true),
    bpEvent($path, 5, 2, "changed", true),
    bpEvent($path, 6, 3, "changed", true),
    bpEvent($path, 7, 4, "changed", true)
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

// Breakpoint 1 should not hit: its condition is false.
// Breakpoint 2 should hit: condition is true.
verifyBpHit(2, 5);
resumeTarget();

// Breakpoint 3 should hit and generate a warning: condition is invalid php
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array(
      "category" => "warning"
  )));

verifyBpHit(3, 6);
resumeTarget();

// Breakpoint 4 should hit and generate a warning: returned non bool
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array(
      "category" => "warning"
  )));
verifyBpHit(4, 7);
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
vsDebugCleanup($testProcess[0], $testProcess[1], $testProcess[2], -1);
