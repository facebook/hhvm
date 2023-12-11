<?hh
<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
$breakpoints = vec[
   dict[
     "path" => __FILE__ . ".test",
     "breakpoints" => vec[
       dict["line" => 9, "calibratedLine" => 9, "condition" => ""],
       dict["line" => 10, "calibratedLine" => 10, "condition" => ""],
       dict["line" => 12, "calibratedLine" => 16, "condition" => "",
        "multiLine" => true],
     ]]
   ];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);
// Skip breakpoint resolution messages.
skipMessages(count($breakpoints[0]{'breakpoints'}));

checkForOutput($testProcess, "Hello world.\n", "stdout");

// Verify we hit breakpoint 1.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);
resumeTarget();

// Verify we hit breakpoint 2.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[1]);
resumeTarget();

// Verify we hit breakpoint 3.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[2]);

// Set a breakpoint past the end of the file. This breakpoint will never verify.
$breakpoints = vec[
  dict[
    "path" => __FILE__ . ".test",
    "breakpoints" => vec[
      dict["line" => 200, "calibratedLine" => 200, "condition" => ""],
    ]]
  ];
setBreakpoints($breakpoints, false);

// Test breakpoint calibration
$breakpoints = vec[
  dict[
    "path" => __FILE__ . ".test",
    "breakpoints" => vec[
      dict["line" => 19, "calibratedLine" => 20, "condition" => ""],
      dict["line" => 22, "calibratedLine" => 26, "condition" => ""],
    ]]
  ];
setBreakpoints($breakpoints, true);

// Setting a breakpoint with an invalid line number should fail.
$setBreakpointsCommand = dict[
  "command" => "setBreakpoints",
  "type" => "request",
  "arguments" => dict[
    "source" =>
      dict[
        "path" => __FILE__ . ".test",
        "name" => "test"
      ],
    "breakpoints" => vec[
        dict["line" => -1, "condition" => ""]
    ]
  ]];
$seq = sendVsCommand($setBreakpointsCommand);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg,
  dict[
    "type" => "response",
    "command" => "setBreakpoints",
    "success" => false,
    "message" => "Breakpoint has invalid line number.",
    "request_seq" => $seq]);

// Remove all breakpoints.
$seq = sendVsCommand(dict[
  "command" => "setBreakpoints",
  "type" => "request",
  "arguments" => dict[
    "source" =>
      dict[
        "path" => __FILE__ . ".test",
        "name" => "test"
      ],
    "lines" => vec[],
    "breakpoints" => vec[]]]);

// Response should indicate no breakpoints remaining.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "setBreakpoints",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
      "breakpoints" => vec[]
  ]]);

$breakpoints = vec[
  dict[
    "path" => __FILE__ . ".test",
    "breakpoints" => vec[
      dict["line" => 29, "calibratedLine" => 29, "condition" => ""],
    ]]
  ];
setBreakpoints($breakpoints, true);
resumeTarget();

checkForOutput($testProcess, "12", "stdout");
checkForOutput($testProcess, "\n", "stdout");

checkForOutput($testProcess, "12", "stdout");
checkForOutput($testProcess, "\n", "stdout");

// Hit the breakpoint inside the loop on line 29
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0], 1);
resumeTarget();

// Hit the breakpoint inside the loop again.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0], 2);

// Remove the breakpoint and resume, it should not hit again now even
// though the loop has 8 more iterations.
$seq = sendVsCommand(dict[
  "command" => "setBreakpoints",
  "type" => "request",
  "arguments" => dict[
    "source" =>
      dict[
        "path" => __FILE__ . ".test",
        "name" => "test"
      ],
    "lines" => vec[],
    "breakpoints" => vec[]]]);

// Response should indicate no breakpoints remaining.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "setBreakpoints",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
      "breakpoints" => vec[]
  ]]);
resumeTarget();

// Verify hard break was hit.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "event",
  "event" => "stopped",
  "body" => dict[
      "threadId" => 1,
      "reason" => "breakpoint",
      "description" => "hphp_debug_break()",
  ]]);

// Verify relative breakpoints work. Two different files named "test.php"
// will be required by the test script, the breakpoint should hit on
// both of them.
$breakpoints = vec[
  dict[
    "path" => "test.php.test",
    "breakpoints" => vec[
      dict["line" => 4, "calibratedLine" => 4, "condition" => ""],
    ]]
  ];
setBreakpoints($breakpoints, false);
resumeTarget();

// Breakpoint verifies when the require call executes in the target.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "event",
  "event" => "breakpoint",
  "body" => dict[
      "reason" => "changed",
  ]]);

checkForOutput($testProcess, "hello\n", "stdout");
verifyBpHit("test.php", $breakpoints[0]{'breakpoints'}[0], 1, true);
resumeTarget();
checkForOutput($testProcess, "world\n", "stdout");

checkForOutput($testProcess, "hello\n", "stdout");
verifyBpHit("test.php", $breakpoints[0]{'breakpoints'}[0], 2, true);

//Verify removing relative breakpoints works
$seq = sendVsCommand(dict[
  "command" => "setBreakpoints",
  "type" => "request",
  "arguments" => dict[
    "source" =>
      dict[
        "path" => "test.php.test",
        "name" => "test"
      ],
    "lines" => vec[],
    "breakpoints" => vec[]]]);

// Response should indicate no breakpoints remaining.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "setBreakpoints",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
      "breakpoints" => vec[]
  ]]);

resumeTarget();
checkForOutput($testProcess, "world\n", "stdout");
checkForOutput($testProcess, "hello\n", "stdout");
checkForOutput($testProcess, "world\n", "stdout");
checkForOutput($testProcess, "hello\n", "stdout");
checkForOutput($testProcess, "world\n", "stdout");

checkForOutput($testProcess, "goodbye.\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
}
