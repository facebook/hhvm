<?hh
require(__DIR__ . '/common.inc');
<<__EntryPoint>> function main(): void {
$breakpoints = [
   array(
     "path" => __FILE__ . ".test",
     "breakpoints" => [
       array("line" => 4, "calibratedLine" => 4, "condition" => "\$a == 2"),
       array("line" => 5, "calibratedLine" => 5, "condition" => "\$a == 1"),
       array("line" => 6, "calibratedLine" => 6, "condition" =>
            "436fsl2$#^%l324;;'"),
       array("line" => 7, "calibratedLine" => 7, "condition" => "\"STRING\""),
     ])
   ];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);

// Breakpoint 1 should not hit: its condition is false.
// Breakpoint 2 should hit: condition is true.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[1]);
resumeTarget();

// Breakpoint 3 should hit and generate a warning: condition is invalid php
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array(
      "category" => "stdout"
  )));
$expectedOutput = "Hit fatal : A semicolon (';') is expected here.";
$result = fread($testProcess[1][1], strlen($expectedOutput));
if ($result !== $expectedOutput) {
  throw new UnexpectedValueException("$result !== $expectedOutput");
}

// Ignore the rest of the stack trace.
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array(
      "category" => "console"
  )));

verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[2]);
resumeTarget();

// Breakpoint 4 should hit and generate a warning: returned non bool
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array(
      "category" => "console"
  )));

verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[3]);
resumeTarget();

checkForOutput($testProcess, "hello world.\n", "stdout", true);
vsDebugCleanup($testProcess);

echo "OK!\n";
}
