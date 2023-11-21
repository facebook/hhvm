<?hh

final class Data {
  public static string $vsDebugData = "";
  public static mixed $client = false;
  public static int $seq = 0;
}

function getNextVsDebugMessage() {
  $vsDebugData = Data::$vsDebugData;
  $client = Data::$client;
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
      Data::$vsDebugData = "";
    } else {
      Data::$vsDebugData = substr($vsDebugData, $pos + 1);
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
  if ($msg === '' && feof(Data::$client)) {
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
  $client = Data::$client;
  $commandMsg{'seq'} = ++Data::$seq;
  fwrite($client, json_encode($commandMsg));
  fwrite($client, chr(0));
  return $commandMsg{'seq'};
}

function resumeTarget() {
  // Resume.
  $contCommand = darray[
    "command" => "continue",
    "type" => "request",
    "threadId" => 1
  ];
  sendVsCommand($contCommand);

  // Target resume event (not thread specific)
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "event",
    "event" => "continued",
  ]);

  // Request resumed event.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "event",
    "event" => "continued",
    "body" => darray[
        "threadId" => 1
    ]]);
}

function verifyBpHit($path, $breakpoint, $hitCount = 1, $relative = false) {
  // Verify breakpoint hit count updated.
  $msg = json_decode(getNextVsDebugMessage(), true);
  if (!$relative) {
    checkObjEqualRecursively($msg, darray[
      "type" => "event",
      "event" => "breakpoint",
      "body" => darray[
          "reason" => "changed",
          "breakpoint" => darray[
            "source" => darray["path" => $path],
            "originalLine" => $breakpoint{'line'},
            "nuclide_hitCount" => $hitCount,
            "verified" => true,
            "endLine" => $breakpoint{'calibratedLine'}
      ]]]);
  } else {
    checkObjEqualRecursively($msg, darray[
      "type" => "event",
      "event" => "breakpoint",
      "body" => darray[
          "reason" => "changed",
          "breakpoint" => darray[
            "originalLine" => $breakpoint{'line'},
            "nuclide_hitCount" => $hitCount,
            "verified" => true,
            "endLine" => $breakpoint{'calibratedLine'}
      ]]]);
  }

  // Verify request stopped.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "event",
    "event" => "stopped",
    "body" => darray[
        "threadId" => 1,
        "allThreadsStopped" => true
    ]]);
}

function getHhvmPath() {
  return HH\__internal\hhvm_binary();
}

function skipMessages($skip) {
  for ($i = 0; $i < $skip; $i++) {
    getNextVsDebugMessage();
  }
}

function checkForOutput($testProcess, $expectedOutput, $expectedCategory, $error = false) {
  $pipes = $testProcess[1];
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray["type" => "event", "event" => "output"]);
  checkObjEqualRecursively($msg{'body'}, darray["output" => $expectedOutput, "category" => $expectedCategory]);

  // Verify the message was actually written to the output stream.
  if (!$error) {
    $result = fread($pipes[1], strlen($expectedOutput));
    if ($result !== $expectedOutput) {
      throw new UnexpectedValueException("$result !== $expectedOutput");
    }
  }
}

function setBreakpoints($breakpoints, $verify = true) {

  foreach ($breakpoints as $bpsForFile) {
    $setBreakpointsCommand = darray[
      "command" => "setBreakpoints",
      "type" => "request",
      "arguments" => darray[
        "source" =>
          darray[
            "path" => $bpsForFile{'path'},
            "name" => "test"
          ],
        "breakpoints" => varray[]
      ]];

    $responseBps = varray[];
    $responseLines = darray[];
    foreach ($bpsForFile{'breakpoints'} as $bp) {
      $setBreakpointsCommand['arguments']['breakpoints'][] = darray["line" => $bp{'line'}, "condition" => $bp{'condition'}];
      $responseBps[] = darray["id" => ++ExtVsdebugCommonInc::$bpId, "verified" => true];
      $responseLines{ExtVsdebugCommonInc::$bpId} = $bp{'calibratedLine'};
    }

    $seq = sendVsCommand($setBreakpointsCommand);

    // Expect a setBreakpoints response
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg,
      darray[
        "type" => "response",
        "command" => "setBreakpoints",
        "success" => true,
        "request_seq" => $seq,
        "body" => darray[
          "breakpoints" => $responseBps]]);

    // Expect a new breakpoint event.
    for ($i = 0; $i < count($responseLines); $i++) {
      $msg = json_decode(getNextVsDebugMessage(), true);
      checkObjEqualRecursively($msg, darray["type" => "event","event" => "breakpoint"]);
      checkObjEqualRecursively($msg{'body'}, darray["reason" => "new"]);
      checkObjEqualRecursively($msg{'body'}{'breakpoint'},
        darray["verified" => false, "line" => $setBreakpointsCommand{'arguments'}{'breakpoints'}[$i]{'line'}]
      );
    }

    // Expect breakpoint verified events.
    if ($verify) {
      for ($i = 0; $i < count($responseLines); $i++) {
        $msg = json_decode(getNextVsDebugMessage(), true);
        checkObjEqualRecursively($msg, darray["type" => "event","event" => "breakpoint"]);
        checkObjEqualRecursively($msg{'body'}, darray["reason" => "changed"]);
        checkObjEqualRecursively($msg{'body'}{'breakpoint'},
          darray[
            "verified" => true,
            "endLine" => $responseLines[$msg{'body'}{'breakpoint'}{'id'}]
          ]
        );
      }
    }
  }
}

function stepCommand($cmd) :mixed{
  $seq = sendVsCommand(darray[
    "command" => $cmd,
    "type" => "request",
    "arguments" => darray["threadId" => 1]]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => $cmd,
    "request_seq" => $seq,
    "success" => true]);

  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "event",
    "event" => "continued",
    "body" => darray[
        "threadId" => 1
    ]]);
}

function verifyStopLine($frames, $line, $file) :mixed{
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "event",
    "event" => "stopped",
    "body" => darray[
        "threadId" => 1
    ]]);

  $seq = sendVsCommand(darray[
    "command" => "stackTrace",
    "type" => "request",
    "arguments" => darray[
      "threadId" => 1
    ]]);

  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "stackTrace",
    "request_seq" => $seq,
    "success" => true,
    "body" => darray[
        "totalFrames" => $frames,
        "stackFrames" => varray[
          darray[
            "source" => darray["path" => $file, "name" => str_replace(".test", "", basename($file))],
            "line" => $line,
          ]
        ]
      ]
    ]);
}

function vsDebugLaunch($scriptPath, $sendDefaultInit = true, $breakpoints = varray[]) {
  $descriptorspec = darray[
     0 => varray["pipe", "r"], // stdin
     1 => varray["pipe", "w"], // stdout
     2 => varray["pipe", "w"]  // stderr
   ];

  // When the client is run in jit mode, enable jit is the debugging hhvm as well.
  $disable_jit = ini_get('hhvm.jit') ? 0 : 1;

  if (!\HH\global_get('USE_UNIX_SOCKET')) {
    // Grab an available port.
    $sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    socket_bind($sock, "localhost", 0) || die("Could not bind socket");
    $ip = null;
    $port = null;
    socket_getsockname($sock, inout $ip, inout $port);
    socket_close($sock);

    $addr = gethostbyname("localhost");
    $uri = "tcp://$addr:$port";
    $cmd = vec[
      getHhvmPath(),
      '-d', 'hhvm.hack.lang.look_for_typechecker=0',
      '--mode', 'vsdebug',
      '--vsDebugPort', (string) $port,
      '-vEval.Jit=true',
      '-vEval.JitDisabledByVSDebug='.$disable_jit,
      '-vEval.JitPGO=false',
      $scriptPath,
    ];
  } else {
    $sockfile = "/tmp/hhvm_unit_test." . getmypid();
    $uri = "unix://" . $sockfile;
    $cmd = vec[
      getHhvmPath(),
      '-d', 'hhvm.hack.lang.look_for_typechecker=0',
      '--mode', 'vsdebug',
      '--vsDebugDomainSocketPath', $sockfile,
      '-vEval.Jit=true',
      '-vEval.JitDisabledByVSDebug='.$disable_jit,
      '-vEval.JitPGO=false',
      $scriptPath,
    ];
  }

  $cmd = $cmd |> array_map($x ==> escapeshellarg($x), $$) |> implode(' ', $$);
  $pipes = null;
  $process = proc_open($cmd, $descriptorspec, inout $pipes);
  if (!is_resource($process)) {
    throw new UnexpectedValueException("Failed to open child process!");
  }

  $vsDebugData = Data::$vsDebugData;
  $client = Data::$client;
  $tries = 0;
  while ($client === false) {
    $errno = null;
    $errorMessage = null;
    $error = error_reporting();
    try {
      error_reporting(0);
      $client = stream_socket_client($uri, inout $errno, inout $errorMessage);
    } catch (Exception $e) {
    }
    error_reporting($error);
    if ($client === false && $tries > 5) {
      proc_terminate($process, 9);
      proc_close($process);
      throw new UnexpectedValueException("Failed to connect: $errorMessage");
    }

    if ($client === false) {
      $tries++;
      sleep(2);
    }
  }

  Data::$client = $client;

  if ($sendDefaultInit) {
    $initCommand = darray[
      "command" => "initialize",
      "type" => "request",
      "seq" => 1,
      "arguments" => darray[
        "clientID" => "Nuclide",
        "adapterID" => "hhvm",
        "linesStartAt1" => true,
        "columnsStartAt1" => true,
        "pathFormat" => "path"]
      ];

    $seq = sendVsCommand($initCommand);

    // Get an InitializeResponse
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg,
      darray[
        "type" => "response",
        "command" => "initialize",
        "success" => true,
        "request_seq" => $seq]);
    checkObjEqualRecursively($msg{'body'},
      darray[
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
      ]);

    $launchCommand = darray[
      "command" => "launch",
      "type" => "request",
      "arguments" => darray[
        "startupDocumentPath" => __DIR__ . '/dummy.php.inc']
      ];

    $seq = sendVsCommand($launchCommand);

    // Expect a launch response
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, darray["type" => "response", "command" => "launch", "success" => true, "request_seq" => $seq]);

    // Expect an initialized event
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, darray["type" => "event", "event" => "initialized"]);

    // Expect an output event about the console being prepared.
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, darray["type" => "event", "event" => "output"]);
    checkObjEqualRecursively($msg{'body'}, darray["category" => "console"]);
    // The exact text of the message is allowed to change, we don't need to enforce that in test code here.
    // But it should not be empty.
    assertIsNotEmpty($msg{'body'}{'output'});

    // Verify once the dummy is started it prints a hello message, so we know it's running.
    $dummyMsg = "I am the dummy!\n";
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, darray["type" => "event", "event" => "output"]);
    checkObjEqualRecursively($msg{'body'}, darray["output" => $dummyMsg, "category" => "stdout"]);

    // Verify that the dummy actually printed its message to stdout successfully, in addition
    // to the debugger informing the client of the output.
    $result = fread($pipes[1], strlen($dummyMsg));
    if ($result !== $dummyMsg) {
      throw new UnexpectedValueException("$result !== $dummyMsg");
    }

    // Now the debugger sends a message telling the user the console is ready,
    // followed by a readyForEvaluations event.
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, darray["type" => "event", "event" => "output"]);
    checkObjEqualRecursively($msg{'body'}, darray["category" => "success"]);

    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, darray["type" => "event", "event" => "readyForEvaluations"]);

    // Set any initial breakpoints that need to be set before we resume the target.
    setBreakpoints($breakpoints, false);

    // Send configuration done, which resumes the target.
    $configDoneCommand = darray[
      "command" => "configurationDone",
      "type" => "request",
      "seq" => 3,
    ];
    $seq = sendVsCommand($configDoneCommand);

    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, darray["type" => "response", "command" => "configurationDone", "success" => true, "request_seq" => $seq]);

    // The debugger sends a contiued event indicating the target resumed.
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, darray["type" => "event", "event" => "continued"]);
    checkObjEqualRecursively($msg{'body'}, darray["allThreadsContinued" => false]);
  }

  return varray[$process, $pipes, $client];
}

// Polls up to 10 seconds waiting for the specified process to exit.
// Returns true if it exited, false otherwise.
function pollForExit($process) {
  $tries = 0;
  $status = null;

  while ($tries < 10) {
    $status = proc_get_status($process);
    if ($status{'running'} !== false) {
     sleep(1);
    } else {
     return true;
    }
    $tries++;
  }

  $status = proc_get_status($process);
  return $status{'running'} === false;
}

function vsDebugCleanup($testProcess, $expectedExitCode = 0) {
  $process = $testProcess[0];
  $pipes = $testProcess[1];
  $client = $testProcess[2];

  // The script should have exited.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, darray[
    "type" => "event",
    "event" => "thread",
    "body" => darray[
      "threadId" => 1,
      "reason" => "exited"
    ]]);

  fclose($client);

  foreach ($pipes as $pipe) {
    fclose($pipe);
  }

  if (!pollForExit($process)) {
    // Ask the process to exit.
    proc_terminate($process);

    if (!pollForExit($process)) {
      // Send process SIGKILL
      proc_terminate($process, 9);
    }
  }

  proc_close($process);
}

abstract final class ExtVsdebugCommonInc {
  public static $bpId = 0;
}

////////////////////////////////////////////
// Common Helpers for Writing Unit Tests
////////////////////////////////////////////

function skip_events() {
  $msg = json_decode(getNextVsDebugMessage(), true);
  while ($msg['type'] == 'event') {
    if ($msg['event'] === 'output') {
      $body = $msg['body'];
      printf("OUT: %s", $body['output']);
    }
    $msg = json_decode(getNextVsDebugMessage(), true);
  }
  return $msg;
}

function send_eval($expr) {
  return sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => $expr,
      'context' => 'repl',
      'frameId' => 2]]);
}

function expect_success($seq) {
  $msg = skip_events();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    "request_seq" => $seq]);
}

function expect_tv($seq, string $type, $val) {
  $msg = skip_events();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    'body' => dict[
      'type' => $type,
      'result' => $val,
    ],
    "request_seq" => $seq]);
}

function expect_int($seq, $i) {
  return expect_tv($seq, 'int', $i);
}
