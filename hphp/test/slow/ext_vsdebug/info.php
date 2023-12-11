<?hh

function getInfo($sym) :mixed{
  $command = dict[
    "command" => "info",
    "type" => "request",
    "arguments" => dict[
      "object" => $sym,
      "threadId" => 1,
    ]];

  $seq = sendVsCommand($command);
  $response = dict[
    "type" => "response",
    "command" => "info",
    "success" => true,
    "request_seq" => $seq,
  ];

  $str = getNextVsDebugMessage();
  $msg = json_decode($str, true);
  checkObjEqualRecursively($msg, $response);
  echo $msg['body']['info'] . "\n";
}
<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
$path = __FILE__ . ".test";

$breakpoints = vec[
   dict[
     "path" => __FILE__ . ".test",
     "breakpoints" => vec[
       dict["line" => 7, "calibratedLine" => 7, "condition" => ""],
       dict["line" => 12, "calibratedLine" => 12, "condition" => ""],
     ]]
   ];

$testProcess = vsDebugLaunch($path, true, $breakpoints);

// Skip breakpoint resolution messages.
skipMessages(count($breakpoints[0]{'breakpoints'}));

// Verify we hit breakpoint 1.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// Expect info for a class named "A"
getInfo('A');

// Expect info for a function named "bar"
getInfo('bar');

// Expect info about the current stop location of thread 1.
getInfo('');

resumeTarget();

// Verify we hit breakpoint 2.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[1]);

// Expect info about the current stop location of thread 1.
getInfo('');

// Expect info about a built-in method
getInfo('strcmp');

// Expect info about a native projected function
getInfo('hphp_debug_break');

// Info about a class variable
getInfo('A::$y');

// Info about a class constant
getInfo('A::CONSTANT');

// Info about a class method
getInfo('A::foo()');

resumeTarget();
vsDebugCleanup($testProcess);

echo "OK!\n";
}
