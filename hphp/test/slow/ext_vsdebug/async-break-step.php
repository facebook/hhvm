<?hh
<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
$breakpoints = varray[
  darray[
    "path" => __FILE__ . ".test",
      "breakpoints" => varray[
        darray["line" => 5, "calibratedLine" => 5, "condition" => ""],
    ]
  ]
];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);
// Skip breakpoint resolution messages.
skipMessages(count($breakpoints[0]{'breakpoints'}));

checkForOutput($testProcess, "bar\n", "stdout");

// Hit breakpoint in foo
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// Set breakpoints after call to foo
$breakpoints = varray[
  darray[
    "path" => __FILE__ . ".test",
      "breakpoints" => varray[
        darray["line" => 17, "calibratedLine" => 17, "condition" => ""],
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
