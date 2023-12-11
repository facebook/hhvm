<?hh
<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
$breakpoints = vec[
  dict[
    "path" => __FILE__ . ".test",
      "breakpoints" => vec[
        dict["line" => 4, "calibratedLine" => 4, "condition" => ""],
    ]
  ]
];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);
// Skip breakpoint resolution messages.
skipMessages(count($breakpoints[0]{'breakpoints'}));

// Hit breakpoint in foo
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// async step-over
stepCommand("next");
checkForOutput($testProcess, "bar\n", "stdout");
verifyStopLine(6, 5, __FILE__.".test");
// Get out of foo using step-over commands
stepCommand("next");
checkForOutput($testProcess, "foo\n", "stdout");
verifyStopLine(6, 6, __FILE__.".test");
stepCommand("next");
verifyStopLine(5, 16, __FILE__.".test");
resumeTarget();

verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0], 2);
// step-out before suspend.
stepCommand("stepOut");
verifyStopLine(5, 21, __FILE__.".test");
resumeTarget();
checkForOutput($testProcess, "bar\n", "stdout");
checkForOutput($testProcess, "foo\n", "stdout");

verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0], 3);
// Set breakpoints after await in foo
$breakpoints = vec[
  dict[
    "path" => __FILE__ . ".test",
      "breakpoints" => vec[
        dict["line" => 5, "calibratedLine" => 5, "condition" => ""],
    ]
  ]
];
setBreakpoints($breakpoints);
resumeTarget();
checkForOutput($testProcess, "bar\n", "stdout");
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);
stepCommand("stepOut");
checkForOutput($testProcess, "foo\n", "stdout");
verifyStopLine(5, 31, __FILE__.".test");
resumeTarget();

checkForOutput($testProcess, "bar\n", "stdout");
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0], 2);
// Set breakpoints after call to foo
$breakpoints = vec[
  dict[
    "path" => __FILE__ . ".test",
      "breakpoints" => vec[
        dict["line" => 38, "calibratedLine" => 38, "condition" => ""],
    ]
  ]
];
setBreakpoints($breakpoints);
resumeTarget();
checkForOutput($testProcess, "foo\n", "stdout");

verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);
resumeTarget();

checkForOutput($testProcess, "Done\n", "stdout");

vsDebugCleanup($testProcess);

echo "OK!\n";
}
