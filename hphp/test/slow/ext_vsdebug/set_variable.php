<?hh
require(__DIR__ . '/common.inc');

$path = __DIR__ . "/set_variable.php.test";
$GLOBALS["path"] = $path;
$GLOBALS["file"] = "set_variable.php.test";

// Calibration mappings:
$mapping[5] = 5;
$mapping[7] = 7;
$mapping[10] = 10;
$mapping[19] = 19;
$GLOBALS['mapping'] = $mapping;

$setBreakpointsCommand = array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 3,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "set_variable.php.test"
      ),
    "lines" => [5],
    "breakpoints" => [
      array("line" => 5, "condition" => "")
    ]
  ));

$setBreakpointsRepsponse = array(
  "type" => "response",
  "command" => "setBreakpoints",
  "success" => true,
  "request_seq" => 3,
  "body" => array(
    "breakpoints" => array(
      array("id" => 1, "verified" => false),
    )));

$testProcess = vsDebugLaunch(
  __DIR__ . '/set_variable.php.test',
  true,
  [$setBreakpointsCommand],
  [
    // Response event
    $setBreakpointsRepsponse,

    // New BP event for each breakpoint
    bpEvent($path, 5, 1, "new", false),

    // Resolved BP event for each bp
    bpEvent($path, 5, 1, "changed", true),
  ]
);

// Verify we resumed from loader break.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued",
  "body" => array(
      "allThreadsContinued" => false
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued",
  "body" => array(
      "allThreadsContinued" => true,
      "threadId" => 1
  )));

// Verify we hit breakpoint 1.
verifyBpHit(1, 5);

sendVsCommand(array(
  "command" => "stackTrace",
  "type" => "request",
  "seq" => 4,
  "arguments" => array(
    "threadId" => 1
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => 4,
  "success" => true,
  "body" => array(
      "totalFrames" => 1,
      "stackFrames" => [
        array(
          "source" => array("path" => $path, "name" => $path),
          "id" => 1,
          "line" => 5,
          "name" => "{main}"
        )
      ]
    )
  ));

// Get scopes.
sendVsCommand(array(
  "command" => "scopes",
  "type" => "request",
  "seq" => 5,
  "arguments" => array("frameId" => 1)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "scopes",
  "request_seq" => 5,
  "success" => true,
  "body" => [
    "scopes" => [
      array(
        "namedVariables" => 6,
        "name" => "Locals",
        "variablesReference" => 2
      ),
  ]]));

// Get locals
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 6,
  "arguments" => array("variablesReference" => 2)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => 6,
  "success" => true,
  "body" => [
    "variables" => [
      array(
        "name" => "\$argc",
      ),
      array(
        "name" => "\$argv",
      ),
      array(
        "name" => "\$HTTP_RAW_POST_DATA",
      ),
      array(
        "type" => "string",
        "name" => "\$str",
        "value" => "hey",
      ),
      array(
        "type" => "int",
        "name" => "\$x",
        "value" => "1",
      ),
      array(
        "type" => "int",
        "name" => "\$y",
        "value" => "2",
      ),
  ]]
  ));

if (count($msg{"body"}{"variables"}) != 6) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Set variable.
sendVsCommand(array(
  "command" => "setVariable",
  "type" => "request",
  "seq" => 7,
  "arguments" => array(
    "variablesReference" => 2,
    "name" => "\$x",
    "value" => "2"
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "setVariable",
  "request_seq" => 7,
  "success" => true,
  "body" => array(
    "value" => "2",
    "type" => "int"
  )));

sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 6,
  "arguments" => array("variablesReference" => 2)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => 6,
  "success" => true,
  "body" => [
    "variables" => [
      array(
        "name" => "\$argc",
      ),
      array(
        "name" => "\$argv",
      ),
      array(
        "name" => "\$HTTP_RAW_POST_DATA",
      ),
      array(
        "type" => "string",
        "name" => "\$str",
        "value" => "hey",
      ),
      array(
        "type" => "int",
        "name" => "\$x",
        "value" => "2",
      ),
      array(
        "type" => "int",
        "name" => "\$y",
        "value" => "2",
      ),
  ]]
  ));

if (count($msg{"body"}{"variables"}) != 6) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Try to set the variable $y to a string, changing its type.
sendVsCommand(array(
  "command" => "setVariable",
  "type" => "request",
  "seq" => 8,
  "arguments" => array(
    "variablesReference" => 2,
    "name" => "\$y",
    "value" => "hello world"
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output"));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "setVariable",
  "request_seq" => 8,
  "success" => false,));


sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 9,
  "arguments" => array("variablesReference" => 2)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array("category" => "error")
));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => 9,
  "success" => true,
  "body" => [
    "variables" => [
      array(
        "name" => "\$argc",
      ),
      array(
        "name" => "\$argv",
      ),
      array(
        "name" => "\$HTTP_RAW_POST_DATA",
      ),
      array(
        "type" => "string",
        "name" => "\$str",
        "value" => "hey",
      ),
      array(
        "type" => "int",
        "name" => "\$x",
        "value" => "2",
      ),
      array(
        "type" => "int",
        "name" => "\$y",
        "value" => "2",
      ),
  ]]
  ));

if (count($msg{"body"}{"variables"}) != 6) {
  throw new UnexpectedValueException("Unexpected variable count");
}


// Set value of a string.
sendVsCommand(array(
  "command" => "setVariable",
  "type" => "request",
  "seq" => 10,
  "arguments" => array(
    "variablesReference" => 2,
    "name" => "\$str",
    "value" => "there"
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "setVariable",
  "request_seq" => 10,
  "success" => true,
  "body" => array(
    "value" => "there",
    "type" => "string"
  )));

sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 11,
  "arguments" => array("variablesReference" => 2)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => 11,
  "success" => true,
  "body" => [
    "variables" => [
      array(
        "name" => "\$argc",
      ),
      array(
        "name" => "\$argv",
      ),
      array(
        "name" => "\$HTTP_RAW_POST_DATA",
      ),
      array(
        "type" => "string",
        "name" => "\$str",
        "value" => "there",
      ),
      array(
        "type" => "int",
        "name" => "\$x",
        "value" => "2",
      ),
      array(
        "type" => "int",
        "name" => "\$y",
        "value" => "2",
      ),
  ]]
  ));

if (count($msg{"body"}{"variables"}) != 6) {
  throw new UnexpectedValueException("Unexpected variable count");
}

sendVsCommand(array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 12,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "set_variable.php.test"
      ),
    "lines" => [7],
    "breakpoints" => [
      array("line" => 7, "condition" => "")
    ]
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);
resumeTarget();

// Hit breakpoint 2.
verifyBpHit(2, 7);


sendVsCommand(array(
  "command" => "stackTrace",
  "type" => "request",
  "seq" => 15,
  "arguments" => array(
    "threadId" => 1
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => 15,
  "success" => true,
  "body" => array(
      "totalFrames" => 1,
      "stackFrames" => [
        array(
          "source" => array("path" => $path, "name" => $path),
          "id" => 11,
          "line" => 7,
          "name" => "{main}"
        )
      ]
    )
  ));

// Get scopes.
sendVsCommand(array(
  "command" => "scopes",
  "type" => "request",
  "seq" => 16,
  "arguments" => array("frameId" => 11)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "scopes",
  "request_seq" => 16,
  "success" => true,
  "body" => [
    "scopes" => [
      array(
        "namedVariables" => 7,
        "name" => "Locals",
        "variablesReference" => 12
      ),
  ]]));

sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 17,
  "arguments" => array("variablesReference" => 12)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => 17,
  "success" => true,
  "body" => [
    "variables" => [
      array(
        "type" => "array",
        "name" => "\$a",
        "value" => "array[3]",
        "indexedVariables" => 3,
        "variablesReference" => 17
      ),
      array(
        "name" => "\$argc",
      ),
      array(
        "name" => "\$argv",
      ),
      array(
        "name" => "\$HTTP_RAW_POST_DATA",
      ),
      array(
        "type" => "string",
        "name" => "\$str",
        "value" => "there",
      ),
      array(
        "type" => "int",
        "name" => "\$x",
        "value" => "2",
      ),
      array(
        "type" => "int",
        "name" => "\$y",
        "value" => "2",
      ),
  ]]
  ));

sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 18,
  "arguments" => array("variablesReference" => 17)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => 18,
  "success" => true,
  "body" => [
    "variables" => [
      array(
        "type" => "int",
        "name" => "0",
        "value" => "1"
      ),
      array(
        "type" => "int",
        "name" => "1",
        "value" => "2"
      ),
      array(
        "type" => "int",
        "name" => "2",
        "value" => "3"
      ),
    ]
  ]));

sendVsCommand(array(
  "command" => "setVariable",
  "type" => "request",
  "seq" => 20,
  "arguments" => array(
    "variablesReference" => 17,
    "name" => "1",
    "value" => "10"
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "setVariable",
  "request_seq" => 20,
  "success" => true,
  "body" => array(
    "value" => "10",
    "type" => "int"
  )));

// Evaluation requests.

// Simple expression
sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "seq" => 19,
  "arguments" =>
  array("frameId" => 11, "expression" => "1 + 1")));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => 19,
  "success" => true,
  "body" => [
    "result" => "2",
    "type" => "int"
  ]));

// Expression referencing local scope
sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "seq" => 19,
  "arguments" =>
  array("frameId" => 11, "expression" => "\$x + 1")));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => 19,
  "success" => true,
  "body" => [
    "result" => "3",
    "type" => "int"
  ]));

// Expression that runs in the dummy context due to invalid frameId
sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "seq" => 19,
  "arguments" =>
  array("frameId" => 5, "expression" => "\$x + 1")));

// Dummy thread start event.
$msg = json_decode(getNextVsDebugMessage(), true);

// Dummy thread exit event.
$msg = json_decode(getNextVsDebugMessage(), true);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => 19,
  "success" => true,
  "body" => [
    "result" => "1",
    "type" => "int"
  ]));

// Expression with side effects.
sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "seq" => 19,
  "arguments" =>
  array("frameId" => 11, "expression" => "\$x++")));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => 19,
  "success" => true,
  "body" => [
    "result" => "2",
    "type" => "int"
  ]));

sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "seq" => 19,
  "arguments" =>
  array("frameId" => 11, "expression" => "\$x")));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => 19,
  "success" => true,
  "body" => [
    "result" => "3",
    "type" => "int"
  ]));

// Expression that invokes a function.
sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "seq" => 19,
  "arguments" =>
  array("frameId" => 11, "expression" => "foo()")));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => 19,
  "success" => true,
  "body" => [
    "result" => "1",
    "type" => "int"
  ]));

// Expression that invokes a function with a breakpoint in it hits bp.
sendVsCommand(array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 20,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "set_variable.php.test"
      ),
    "lines" => [10, 19],
    "breakpoints" => [
      array("line" => 10, "condition" => ""),
      array("line" => 19, "condition" => "")
    ]
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);

sendVsCommand(array(
  "command" => "evaluate",
  "type" => "request",
  "seq" => 19,
  "arguments" =>
  array("frameId" => 11, "expression" => "foo()")));

// Bp hit event
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "stopped",
  "body" => array(
      "threadId" => 1
  )));

// Verify breakpoint hit count updated.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "breakpoint",
  "body" => array(
      "reason" => "changed",
      "breakpoint" => array(
        "source" => array("path" => $path),
        "originalLine" => 10,
        "id" => 3,
        "nuclide_hitCount" => 1,
        "verified" => true,
        "line" => 10
  ))));

// Verify request stopped.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "stopped",
  "body" => array(
      "threadId" => 1,
      "allThreadsStopped" => true
  )));

// Resume from inner bp.
sendVsCommand(array(
  "command" => "continue",
  "type" => "request",
  "seq" => 100,
  "threadId" => 1
));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued",
  "body" => array(
      "allThreadsContinued" => false,
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued",
  "body" => array(
      "allThreadsContinued" => false,
  )));

// Stopped at outer bp again.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "stopped",
  "body" => array(
      "reason" => "Evaluation returned",
      )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "evaluate",
  "request_seq" => 19,
  "success" => true,
  "body" => [
    "result" => "1",
    "type" => "int"
  ]));

resumeTarget();

verifyBpHit(4, 19);

// Set a proprety on an object $obj from line 19.
// We need to refetch stack and scopes so we can get a server ID
// assigned to $obj.
sendVsCommand(array(
  "command" => "stackTrace",
  "type" => "request",
  "seq" => 20,
  "arguments" => array(
    "threadId" => 1
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
sendVsCommand(array(
  "command" => "scopes",
  "type" => "request",
  "seq" => 22,
  "arguments" => array("frameId" => 19)));
$msg = json_decode(getNextVsDebugMessage(), true);
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 23,
  "arguments" => array("variablesReference" => 20)));
$msg = json_decode(getNextVsDebugMessage(), true);
sendVsCommand(array(
  "command" => "setVariable",
  "type" => "request",
  "seq" => 24,
  "arguments" => array(
    "variablesReference" => 27,
    "name" => "\$A",
    "value" => "1000"
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "setVariable",
  "request_seq" => 24,
  "success" => true,
  "body" => array(
    "value" => "1000",
    "type" => "int"
  )));
resumeTarget();

// Verify that the script exited.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "thread",
  "body" => array(
    "threadId" => 1,
    "reason" => "exited"
  )));

// Read anything left it stdout and stderr and echo it.
$stdout = $testProcess[1][1];
$stderr = $testProcess[1][2];
echo stream_get_contents($stdout);
echo stream_get_contents($stderr);
vsDebugCleanup($testProcess[0], $testProcess[1], $testProcess[2]);
