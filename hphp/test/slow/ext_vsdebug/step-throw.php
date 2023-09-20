<?hh

<<__EntryPoint>> function main() :mixed{
require(__DIR__ . '/common.inc');

// Set breakpoint on throw statement
$breakpoints = varray[
   darray[
     "path" => __FILE__ . ".test",
     "breakpoints" => varray[
       darray["line" => 6, "calibratedLine" => 6, "condition" => ""],
     ]]
   ];

$testProcess = vsDebugLaunch(__FILE__.".test", true, $breakpoints);

// Skip breakpoint resolution messages.
skipMessages(count($breakpoints[0]{'breakpoints'}));

// Verify we hit breakpoint
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// Step over should stop at the catch block
stepCommand("next");
verifyStopLine(1, 19, __FILE__.".test");
stepCommand("next");
verifyStopLine(1, 18, __FILE__.".test");

resumeTarget();

checkForOutput($testProcess, "Caught\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
}

