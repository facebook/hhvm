<?hh
abstract final class ExtVsdebugStep {
  public static $path;
}
function verifyStopLine($frames, $line) :mixed{


  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "event",
    "event" => "stopped",
    "body" => darray[
        "threadId" => 1
    ]]);

  $seq = sendVsCommand(darray[
    "command" => "stackTrace",
    "type" => "request",
    "arguments" => darray[
      "threadId" => 1
    ]]);

  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "stackTrace",
    "request_seq" => $seq,
    "success" => true,
    "body" => darray[
        "totalFrames" => $frames,
        "stackFrames" => varray[
          darray[
            "source" => darray["path" => ExtVsdebugStep::$path, "name" => str_replace(".test", "", basename(ExtVsdebugStep::$path))],
            "line" => $line,
          ]
        ]
      ]
    ]);
}

function stepCommand($cmd) :mixed{
  $seq = sendVsCommand(darray[
    "command" => $cmd,
    "type" => "request",
    "arguments" => darray["threadId" => 1]]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => $cmd,
    "request_seq" => $seq,
    "success" => true]);

  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "event",
    "event" => "continued",
    "body" => darray[
        "threadId" => 1
    ]]);
}
<<__EntryPoint>> function main() :mixed{
require(__DIR__ . '/common.inc');

ExtVsdebugStep::$path = __FILE__ . ".test";

$breakpoints = varray[
   darray[
     "path" => __FILE__ . ".test",
     "breakpoints" => varray[
       darray["line" => 9, "calibratedLine" => 9, "condition" => ""],
       darray["line" => 13, "calibratedLine" => 13, "condition" => ""],
     ]]
   ];

$testProcess = vsDebugLaunch(ExtVsdebugStep::$path, true, $breakpoints);

// Skip breakpoint resolution messages.
skipMessages(count($breakpoints[0]{'breakpoints'}));

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
$seq = sendVsCommand(darray[
  "command" => "fb_continueToLocation",
  "type" => "request",
  "seq" => 5,
  "arguments" =>
    darray[
      "threadId" => 1,
      "source" => darray["path" => ExtVsdebugStep::$path],
      "line" => 15
    ]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, darray[
  "type" => "event",
  "event" => "output",
  "body" => darray[
      "category" => "info",
  ]]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, darray[
  "type" => "response",
  "command" => "fb_continueToLocation",
  "request_seq" => $seq]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, darray[
  "type" => "event",
  "event" => "continued"]);

checkForOutput($testProcess, "hello world 2\n", "stdout");
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[1]);

// Resume and exit.
resumeTarget();


checkForOutput($testProcess, "hello world 3\n", "stdout");
checkForOutput($testProcess, "hello world 4\n", "stdout");
checkForOutput($testProcess, "hello world 5\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
}

