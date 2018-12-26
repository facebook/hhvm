<?hh
require(__DIR__ . '/common.inc');

$path = __FILE__ . ".test";
$breakpoints = [
   array(
     "path" => $path,
     "breakpoints" => [
       array("line" => 14, "calibratedLine" => 14, "condition" => ""),
     ])
   ];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);

// Verify we hit breakpoint 1.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// Check thread stacks.
$seq = sendVsCommand(array(
  "command" => "stackTrace",
  "type" => "request",
  "arguments" => array(
    "threadId" => 1
  )
));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => $seq,
  "success" => true,
  "body" => array(
      "totalFrames" => 2,
      "stackFrames" => [
        array(
          "source" => array("path" => $path, "name" => str_replace(".test", "", basename($path))),
          "id" => 1,
          "line" => 14,
          "name" => "innerFunc"
        ),
        array(
          "source" => array("path" => $path, "name" => str_replace(".test", "", basename($path))),
          "id" => 2,
          "line" => 17,
          "name" => "{main}"
        )
      ]
    )
  )
);

// Check threads.
$seq = sendVsCommand(array(
  "command" => "threads",
  "type" => "request"));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "threads",
  "request_seq" => $seq,
  "success" => true,
  "body" => [
    "threads" => [array("name" => "Request 1", "id" => 1)]
  ]
  ));

// Get scopes.
$seq = sendVsCommand(array(
  "command" => "scopes",
  "type" => "request",
  "arguments" => array("frameId" => 1)));
$msg = json_decode(getNextVsDebugMessage(), true);

checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "scopes",
  "request_seq" => $seq,
  "success" => true,
  "body" => [
    "scopes" => [
      array(
        "namedVariables" => 1,
        "name" => "Locals",
      ),
      array(
        "namedVariables" => 9,
        "name" => "Superglobals",
      ),
      array(
        "namedVariables" => 2,
        "name" => "Constants",
      )
  ]]
  ));

// Get locals, only $a should be visible right here.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 3)));
$msg = json_decode(getNextVsDebugMessage(), true);

if (isset($msg['body']['variables'][0]['variablesReference'])) {
  throw new ErrorException('A class with a __toDebugDisplay method should not return variablesReference');
}

checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => $seq,
  "success" => true,
  "body" => [
    "variables" => [
      array(
        "type" => "A",
        "name" => "\$a",
        "value" => "A(42)",
      ),
  ]]
));

resumeTarget();

checkForOutput($testProcess, "lol\n", "stdout");
checkForOutput($testProcess, "hello world 2\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
