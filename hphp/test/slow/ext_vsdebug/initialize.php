<?hh
require(__DIR__ . '/common.inc');
$testProcess = vsDebugLaunch(__DIR__ . '/initialize.php.test', false);

/*
 * Initialize test: Check response of initialization command and reported
 * capabilities.
 */
echo "OK.\n";

$initCommand = array(
  "command" => "initialize",
  "type" => "request",
  "seq" => 1,
  "arguments" => array(
    "clientID" => "Nuclide",
    "adapterID" => "hhvm",
    "linesStartAt1" => true,
    "columnsStartAt1" => true,
    "pathFormat" => "path")
  );
sendVsCommand($initCommand);

// Get an InitializeResponse
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg,
  array(
    "type" => "response",
    "command" => "initialize",
    "success" => true,
    "request_seq" => 1));
checkObjEqualRecursively($msg{'body'},
  array(
    "supportsLoadedSourcesRequest" => false,
    "supportTerminateDebuggee" => false,
    "supportsExceptionOptions" => true,
    "supportsModulesRequest" => false,
    "supportsHitConditionalBreakpoints" => false,
    "supportsConfigurationDoneRequest" => true,
    "supportsDelayedStackTraceLoading" => true,
    "supportsSetVariable" => true,
    "supportsGotoTargetsRequest" => false,
    "supportsExceptionInfoRequest" => true,
    "supportsValueFormattingOptions" => true,
    "supportsEvaluateForHovers" => true,
    "supportsRestartRequest" => false,
    "supportsConditionalBreakpoints" => true,
    "supportsStepBack" => false,
    "supportsCompletionsRequest" => true,
    "supportsRestartFrame" => false,
    "supportsStepInTargetsRequest" => false
  ));

$launchCommand = array(
  "command" => "launch",
  "type" => "request",
  "seq" => 2,
  "arguments" => array(
    "startupDocumentPath" => __DIR__ . '/dummy.php.inc')
  );

sendVsCommand($launchCommand);

$configDoneCommand = array(
  "command" => "configurationDone",
  "type" => "request",
  "seq" => 3,
);
sendVsCommand($configDoneCommand);

$stdout = $testProcess[1][1];
$stderr = $testProcess[1][2];
@stream_get_contents($stdout);
@stream_get_contents($stderr);
vsDebugCleanup($testProcess[0], $testProcess[1], $testProcess[2]);
