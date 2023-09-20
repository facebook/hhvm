<?hh
abstract final class ExtVsdebugStep {
  public static $path;
}

<<__EntryPoint>> function main() :mixed{
require(__DIR__ . '/common.inc');

ExtVsdebugStep::$path = __FILE__ . ".test";

$breakpoints = varray[
   darray[
     "path" => __FILE__ . ".test",
     "breakpoints" => varray[
       darray["line" => 17, "calibratedLine" => 17, "condition" => ""],
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
verifyStopLine(1, 18, __FILE__.".test");

// Step in
stepCommand("stepIn");
verifyStopLine(2, 11, __FILE__.".test");

// Step over.
stepCommand("next");
verifyStopLine(2, 12, __FILE__.".test");

// Step out.
stepCommand("stepOut");
verifyStopLine(1, 18, __FILE__.".test");

// Step over function.
stepCommand("next");
verifyStopLine(1, 19, __FILE__.".test");
stepCommand("next");
verifyStopLine(1, 20, __FILE__.".test");

// Set breakpoint in baz
$breakpoints = vec[
  dict[
    "path" => __FILE__.".test",
    "breakpoints" => vec[
      dict["line" => 4, "calibratedLine" => 4, "condition" => ""],
    ],
  ],
];
setBreakpoints($breakpoints);
resumeTarget();

// Hit breakpoint on baz
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// Set breakpoint on main after returning from the current function
$breakpoints = vec[
  dict[
    "path" => __FILE__.".test",
    "breakpoints" => vec[
      dict["line" => 21, "calibratedLine" => 21, "condition" => ""],
    ],
  ],
];
setBreakpoints($breakpoints);

// Step out to bar where no breakpoint was set
stepCommand("stepOut");
verifyStopLine(3, 7, __FILE__.".test");
resumeTarget();

verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);
resumeTarget();

checkForOutput($testProcess, "hello world 15\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
}

