<?hh

// Test globals.
$GLOBALS['vsDebugData'] = "";
$GLOBALS['client'] = false;
$GLOBALS['seq'] = 0;

function getNextVsDebugMessage() {
  $vsDebugData = $GLOBALS['vsDebugData'];
  $client = $GLOBALS['client'];
  if ($client === false) {
    return null;
  }

  while (true) {
    $pos = strlen($vsDebugData);
    while (true) {
      $result = fread($client, 1);
      if ($result === false || strlen($result) === 0) {
        break;
      }
      $pos++;
      $vsDebugData .= $result;
      if ($result[0] === chr(0)) {
        break;
      }
    }

    if ($pos == 0 && $vsDebugData === "") {
      if (feof($client)) {
        throw new ErrorException("Target has exited.");
      }
      throw new UnexpectedValueException($vsDebugData);
    }

    $data = substr($vsDebugData, 0, $pos - 1);
    if ($pos == strlen($vsDebugData)) {
      $GLOBALS['vsDebugData'] = "";
    } else {
      $GLOBALS['vsDebugData'] = substr($vsDebugData, $pos + 1);
    }
    return $data;
  }
}

function checkArrayEqualRecursively($a1, $a2) {
  foreach ($a1 as $key => $value) {
    if (is_array($value)) {
      checkArrayEqualRecursively($value, $a2[$key]);
    } else {
      if ($value !== $a2[$key]) {
        throw new UnexpectedValueException($value . " !== " . $a2[$key]);
      }
    }
  }
}

function assertIsNotEmpty($val) {
  if ($val == null || $val === '') {
    throw new UnexpectedValueException("Value is empty");
  }
}

function checkObjEqualRecursively($msg, $expected) {
  if ($msg === '' && feof($GLOBALS['client'])) {
    throw new ErrorException("Target has exited.");
  }

  foreach ($expected as $key => $value) {
    $expectedValue = $value;
    $actualValue = $msg{$key};

    if (is_array($expectedValue)) {
      checkArrayEqualRecursively($expectedValue, $actualValue);
    } else if ($actualValue !== $expectedValue) {
      echo json_encode($msg) . "\n";
      throw new UnexpectedValueException("$key: $actualValue != $expectedValue");
    }
  }

  return true;
}

function sendVsCommand($commandMsg) {
  $client = $GLOBALS['client'];
  $commandMsg{'seq'} = ++$GLOBALS['seq'];
  fwrite($client, json_encode($commandMsg));
  fwrite($client, chr(0));
  return $commandMsg{'seq'};
}

function resumeTarget() {
  // Resume.
  $contCommand = array(
    "command" => "continue",
    "type" => "request",
    "threadId" => 1
  );
  sendVsCommand($contCommand);

  // Target resume event (not thread specific)
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "continued",
  ));

  // Request resumed event.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "continued",
    "body" => array(
        "threadId" => 1
    )));
}

function verifyBpHit($path, $breakpoint, $hitCount = 1) {
  // Verify breakpoint hit count updated.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "breakpoint",
    "body" => array(
        "reason" => "changed",
        "breakpoint" => array(
          "source" => array("path" => $path),
          "originalLine" => $breakpoint{'line'},
          "nuclide_hitCount" => $hitCount,
          "verified" => true,
          "endLine" => $breakpoint{'calibratedLine'}
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
}

function getHhvmPath() {
  return PHP_BINARY;
}

function checkForOutput($testProcess, $expectedOutput, $expectedCategory, $error = false) {
  $pipes = $testProcess[1];
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array("type" => "event", "event" => "output"));
  checkObjEqualRecursively($msg{'body'}, array("output" => $expectedOutput, "category" => $expectedCategory));

  // Verify the message was actually written to the output stream.
  if (!$error) {
    $result = fread($pipes[1], strlen($expectedOutput));
    if ($result !== $expectedOutput) {
      throw new UnexpectedValueException("$result !== $expectedOutput");
    }
  }
}

$bpId = 0;
function setBreakpoints($breakpoints, $verify = true) {
  global $bpId;
  foreach ($breakpoints as $bpsForFile) {
    $setBreakpointsCommand = array(
      "command" => "setBreakpoints",
      "type" => "request",
      "arguments" => array(
        "source" =>
          array(
            "path" => $bpsForFile{'path'},
            "name" => "test"
          ),
        "breakpoints" => []
      ));

    $responseBps = [];
    $responseLines = [];
    foreach ($bpsForFile{'breakpoints'} as $bp) {
      array_push(&$setBreakpointsCommand{'arguments'}{'breakpoints'},
          array("line" => $bp{'line'}, "condition" => $bp{'condition'}));
      array_push(&$responseBps, array("id" => ++$bpId, "verified" => false));
      $responseLines{$bpId} = $bp{'calibratedLine'};
    }

    $seq = sendVsCommand($setBreakpointsCommand);

    // Expect a setBreakpoints response
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg,
      array(
        "type" => "response",
        "command" => "setBreakpoints",
        "success" => true,
        "request_seq" => $seq,
        "body" => array(
          "breakpoints" => $responseBps)));

    // Expect a new breakpoint event.
    for ($i = 0; $i < count($responseLines); $i++) {
      $msg = json_decode(getNextVsDebugMessage(), true);
      checkObjEqualRecursively($msg, array("type" => "event","event" => "breakpoint"));
      checkObjEqualRecursively($msg{'body'}, array("reason" => "new"));
      checkObjEqualRecursively($msg{'body'}{'breakpoint'},
        array("verified" => false, "line" => $setBreakpointsCommand{'arguments'}{'breakpoints'}[$i]{'line'})
      );
    }

    // Expect breakpoint verified events.
    if ($verify) {
      for ($i = 0; $i < count($responseLines); $i++) {
        $msg = json_decode(getNextVsDebugMessage(), true);
        checkObjEqualRecursively($msg, array("type" => "event","event" => "breakpoint"));
        checkObjEqualRecursively($msg{'body'}, array("reason" => "changed"));
        checkObjEqualRecursively($msg{'body'}{'breakpoint'},
          array(
            "verified" => true,
            "endLine" => $responseLines[$msg{'body'}{'breakpoint'}{'id'}]
          )
        );
      }
    }
  }
}

function vsDebugLaunch($scriptPath, $sendDefaultInit = true, $breakpoints = []) {
  $descriptorspec = array(
     0 => array("pipe", "r"), // stdin
     1 => array("pipe", "w"), // stdout
     2 => array("pipe", "w")  // stderr
  );

  // Grab an available port.
  $sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
  socket_bind($sock, "localhost", 0) or die("Could not bind socket");
  socket_getsockname($sock, &$ip, &$port);
  socket_close($sock);

  $addr = gethostbyname("localhost");
  $cmd = vec[
    getHhvmPath(),
    '-d', 'hhvm.hack.lang.look_for_typechecker=0',
    '--mode', 'vsdebug',
    '--vsDebugPort', (string) $port,
    $scriptPath,
  ] |> array_map($x ==> escapeshellarg($x), $$) |> implode(' ', $$);
  $process = proc_open($cmd, $descriptorspec, &$pipes);
  if (!is_resource($process)) {
    throw new UnexpectedValueException("Failed to open child process!");
  }

  $vsDebugData = $GLOBALS['vsDebugData'];
  $client = $GLOBALS['client'];
  $tries = 0;
  while ($client === false) {
    try {
      $client = @stream_socket_client("tcp://$addr:$port", $errno, $errorMessage);
    } catch (Exception $e) {
    }
    if ($client === false && $tries > 5) {
      throw new UnexpectedValueException("Failed to connect: $errorMessage");
    }

    if ($client === false) {
      $tries++;
      sleep(1);
    }
  }

  $GLOBALS['client'] = $client;

  if ($sendDefaultInit) {
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

    $seq = sendVsCommand($initCommand);

    // Get an InitializeResponse
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg,
      array(
        "type" => "response",
        "command" => "initialize",
        "success" => true,
        "request_seq" => $seq));
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
        "supportsExceptionInfoRequest" => false,
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
      "arguments" => array(
        "startupDocumentPath" => __DIR__ . '/dummy.php.inc')
      );

    $seq = sendVsCommand($launchCommand);

    // Expect a launch response
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "response", "command" => "launch", "success" => true, "request_seq" => $seq));

    // Expect an initialized event
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "event", "event" => "initialized"));

    // Expect an output event about the console being prepared.
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "event", "event" => "output"));
    checkObjEqualRecursively($msg{'body'}, array("category" => "console"));
    // The exact text of the message is allowed to change, we don't need to enforce that in test code here.
    // But it should not be empty.
    assertIsNotEmpty($msg{'body'}{'output'});

    // Verify once the dummy is started it prints a hello message, so we know it's running.
    $dummyMsg = "I am the dummy!\n";
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "event", "event" => "output"));
    checkObjEqualRecursively($msg{'body'}, array("output" => $dummyMsg, "category" => "stdout"));

    // Verify that the dummy actually printed its message to stdout successfully, in addition
    // to the debugger informing the client of the output.
    $result = fread($pipes[1], strlen($dummyMsg));
    if ($result !== $dummyMsg) {
      throw new UnexpectedValueException("$result !== $dummyMsg");
    }

    // Now the debugger sends a message telling the user the console is ready,
    // followed by a readyForEvaluations event.
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "event", "event" => "output"));
    checkObjEqualRecursively($msg{'body'}, array("category" => "success"));

    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "event", "event" => "readyForEvaluations"));

    // Set any initial breakpoints that need to be set before we resume the target.
    setBreakpoints($breakpoints);

    // Send configuration done, which resumes the target.
    $configDoneCommand = array(
      "command" => "configurationDone",
      "type" => "request",
      "seq" => 3,
    );
    $seq = sendVsCommand($configDoneCommand);

    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "response", "command" => "configurationDone", "success" => true, "request_seq" => $seq));

    // The debugger sends a contiued event indicating the target resumed.
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "event", "event" => "continued"));
    checkObjEqualRecursively($msg{'body'}, array("allThreadsContinued" => false));
  }

  return [$process, $pipes, $client];
}

function vsDebugCleanup($testProcess, $expectedExitCode = 0) {
  $process = $testProcess[0];
  $pipes = $testProcess[1];
  $client = $testProcess[2];

  // The script should have exited.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "thread",
    "body" => array(
      "threadId" => 1,
      "reason" => "exited"
    )));

  fclose($client);

  foreach ($pipes as $pipe) {
    fclose($pipe);
  }

  $tries = 0;
  $status = null;

  while ($tries < 10) {
    $status = proc_get_status($process);
    if ($status{'running'} !== false) {
      sleep(1);
    } else {
      break;
    }
    $tries++;
  }

  if ($status{'running'} !== false) {
    throw new UnexpectedValueException("Expected child process to have exited!");
  }

  proc_close($process);
}
