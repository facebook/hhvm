<?hh
require(__DIR__ . '/common.inc');
$testProcess = vsDebugLaunch(__DIR__ . '/launch.php.test', false);

/*
 * Dummy test: Verify the dummy starts up and a function defined inside the
 * dummy is visible to the console REPL.
 */

$seq = 1;
$initCommand = array(
 "command" => "initialize",
 "type" => "request",
 "seq" => ++$seq,
 "arguments" => array(
   "clientID" => "Nuclide",
   "adapterID" => "hhvm",
   "linesStartAt1" => true,
   "columnsStartAt1" => true,
   "pathFormat" => "path")
 );
sendVsCommand($initCommand);

$launchCommand = array(
 "command" => "launch",
 "type" => "request",
 "seq" => ++$seq,
 "arguments" => array(
   "startupDocumentPath" => __DIR__ . '/dummy.php.inc')
 );

sendVsCommand($launchCommand);

// The expected "preparing..." message. It's fine to change this in HHVM,
// if changing update the following line.
$expectedPrepareOutput = "Preparing your Hack/PHP console. Please wait...";
$msg = null;
while (true) {
  $msg = json_decode(getNextVsDebugMessage(), true);
  if ($msg{'type'} === "event" && $msg{'event'} === "output") {
    break;
  }

  if ($msg == null) {
    throw new UnexpectedValueException("Didn't find output event message");
  }
}

checkObjEqualRecursively($msg, array("type" => "event", "event" => "output"));
checkObjEqualRecursively($msg{'body'},
  array("output" => $expectedPrepareOutput, "category" => "warning"));

$expectedReadyOutput = "The Hack/PHP console is now ready to use.";
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array("type" => "event", "event" => "output"));
checkObjEqualRecursively($msg{'body'},
  array("output" => $expectedReadyOutput, "category" => "success"));

// Ask the dummy request to evaluate dummyFoo();
// dummyFoo() echos a string and returns 1.
$evalCommand = array(
  "command" => "evaluate",
  "type" => "request",
  "seq" => ++$seq,
  "arguments" => array(
    "expression" => "dummyFoo();",
    "frameId" => -1
  ));

sendVsCommand($evalCommand);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg,
  array(
    "type" => "response",
    "command" => "evaluate",
    "request_seq" => $seq,
    "success" => true,
  ));
checkObjEqualRecursively($msg{'body'},
  array(
    "type" => "int",
    "result" => "1"
  ));

$configDoneCommand = array(
 "command" => "configurationDone",
 "type" => "request",
 "seq" => ++$seq,
);
sendVsCommand($configDoneCommand);

// Read anything left it stdout and stderr and echo it.
vsDebugCleanup($testProcess[0], $testProcess[1], $testProcess[2]);
