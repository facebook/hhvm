<?hh

<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
$breakpoints = vec[
  dict[
    "path" => __FILE__ . ".test",
    "breakpoints" => vec[
      dict["line" => 6, "calibratedLine" => 6, "condition" => ""],
    ]
  ]
];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);
// Skip breakpoint resolution messages.
skipMessages(count($breakpoints[0]{'breakpoints'}));

verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);
$breakpoints = vec[
  dict[
    "path" => "MyClass.inc",
    "breakpoints" => vec[
      dict["line" => 5, "calibratedLine" => 5, "condition" => ""],
    ]
  ]
];
setBreakpoints($breakpoints);
resumeTarget();

verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0], 1, true);
resumeTarget();

checkForOutput($testProcess, "OK\n", "stdout");
}
