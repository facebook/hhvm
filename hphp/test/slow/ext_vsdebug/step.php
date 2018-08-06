<?hh
require(__DIR__ . '/common.inc');

$path = __FILE__ . ".test";

function verifyStopLine($frames, $line) {
  global $path;

  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "stopped",
    "body" => array(
        "threadId" => 1
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
  $seq = sendVsCommand(array(
    "command" => $cmd,
    "type" => "request",
    "arguments" => array("threadId" => 1)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => $cmd,
    "request_seq" => $seq,
    "success" => true));

  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "continued",
    "body" => array(
        "threadId" => 1
    )));
}


$breakpoints = [
   array(
     "path" => __FILE__ . ".test",
     "breakpoints" => [
       array("line" => 9, "calibratedLine" => 9, "condition" => ""),
       array("line" => 13, "calibratedLine" => 13, "condition" => ""),
     ])
   ];

$testProcess = vsDebugLaunch($path, true, $breakpoints);

// Verify we hit breakpoint 1.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// Step over.
stepCommand("next");
checkForOutput($testProcess, "hello world 1\n", "stdout");
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
$seq = sendVsCommand(array(
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
  "request_seq" => $seq));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued"));

checkForOutput($testProcess, "hello world 2\n", "stdout");
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[1]);

// Resume and exit.
resumeTarget();


checkForOutput($testProcess, "hello world 3\n", "stdout");
checkForOutput($testProcess, "hello world 4\n", "stdout");
checkForOutput($testProcess, "hello world 5\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
