<?hh
require(__DIR__ . '/common.inc');

$path = __DIR__ . "/exception_bp.php.test";
$GLOBALS["path"] = $path;
$GLOBALS["file"] = "exception_bp.php.test";
$GLOBALS['mapping'] = [];

$exnBpCommand = array(
  "command" => "setExceptionBreakpoints",
  "type" => "request",
  "seq" => 3,
  "arguments" => array(
    "exceptionOptions" => array(
      "breakMode" => "always"
    )
  ));

$exnBpResp = array(
  "type" => "response",
  "command" => "setExceptionBreakpoints",
  "success" => true,
  "request_seq" => 3,);

$testProcess = vsDebugLaunch(
  __DIR__ . '/exception_bp.php.test',
  true,
  [$exnBpCommand],
  [$exnBpResp]
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

// Verify we stopped on exception thrown. There will be 2 stop messages,
// one when the target stops and one when the thread enters its command queue.
$exnStopObj = array(
  "type" => "event",
  "event" => "stopped",
  "body" => array(
      "threadId" => 1,
      "reason" => "Exception (UnexpectedValueException) thrown"
  ));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $exnStopObj);
$exnStopObj{"body"}{"allThreadsStopped"} = true;
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $exnStopObj);

// Set exn breaks to an invalid value.
$exnBpCommand = array(
  "command" => "setExceptionBreakpoints",
  "type" => "request",
  "seq" => 4,
  "arguments" => array(
    "exceptionOptions" => array(
      "breakMode" => "INVALID"
    )
  ));
sendVsCommand($exnBpCommand);

$exnBpResp = array(
  "type" => "response",
  "command" => "setExceptionBreakpoints",
  "success" => false,
  "request_seq" => 4);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $exnBpResp);

// Invalid exception break mode should tell the user.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array(
      "category" => "error"
  )));

// Set to break on unhandled only - this currently generates a warning on
// exception but doesn't break.
$exnBpCommand = array(
  "command" => "setExceptionBreakpoints",
  "type" => "request",
  "seq" => 5,
  "arguments" => array(
    "exceptionOptions" => array(
      "breakMode" => "unhandled"
    )
  ));
sendVsCommand($exnBpCommand);

$exnBpResp = array(
  "type" => "response",
  "command" => "setExceptionBreakpoints",
  "success" => true,
  "request_seq" => 5);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $exnBpResp);

// Set a breakpoint on line 12.
sendVsCommand(array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 6,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "exception_bp.php.test"
      ),
    "lines" => [12],
    "breakpoints" => [
      array("line" => 12, "condition" => "")
    ]
  )));
// Set bp response, new bp, verified bp
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);

resumeTarget();

// See a warning when the next exception is thrown.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "output",
  "body" => array(
    "category" => "warning"
  )));

// Hit breakpoint on line 12.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "stopped",
  "body" => array(
    "reason" => "Breakpoint 1 (exception_bp.php.test:12)"
  )));
// BP hit count changed.
$msg = json_decode(getNextVsDebugMessage(), true);
$msg = json_decode(getNextVsDebugMessage(), true);

// Turn off break on exceptions.
$exnBpCommand = array(
  "command" => "setExceptionBreakpoints",
  "type" => "request",
  "seq" => 7,
  "arguments" => array(
    "exceptionOptions" => array(
      "breakMode" => "never"
    )
  ));
sendVsCommand($exnBpCommand);

$exnBpResp = array(
  "type" => "response",
  "command" => "setExceptionBreakpoints",
  "success" => true,
  "request_seq" => 7);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, $exnBpResp);

// Resume the target, break on exn is not enabled so the target should
// throw and exit.
resumeAndCleanup($testProcess);
