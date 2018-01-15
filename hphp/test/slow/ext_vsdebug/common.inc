<?hh

// Test globals.
$GLOBALS['vsDebugData'] = "";
$GLOBALS['client'] = false;

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

function checkObjEqualRecursively($msg, $expected) {
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
  fwrite($client, json_encode($commandMsg));
  fwrite($client, chr(0));
}

function resumeTarget() {
  // Resume.
  $contCommand = array(
    "command" => "continue",
    "type" => "request",
    "seq" => 100,
    "threadId" => 1
  );
  sendVsCommand($contCommand);

  // Target resume event (not thread specific)
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "continued",
    "body" => array(
        "allThreadsContinued" => false,
    )));

  // Request resumed event.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
    "type" => "event",
    "event" => "continued",
    "body" => array(
        "allThreadsContinued" => true,
        "threadId" => 1
    )));
}

function bpEvent($path, $line, $id, $reason, $verified) {
  return array(
    "type" => "event",
    "event" => "breakpoint",
    "body" => array(
      "reason" => $reason,
      "breakpoint" => array(
        "source" => array(
            "path" => $path,
        ),
        "verified" => $verified,
        "column" => 0
    )));
}

function verifyBpHit($id, $line, $hitCount = 1) {
  global $path;
  global $mapping;
  global $file;

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
          "originalLine" => $line,
          "id" => $id,
          "nuclide_hitCount" => $hitCount,
          "verified" => true,
          "line" => $mapping[$line]
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

function vsDebugLaunch($scriptPath, $sendDefaultInit = true, $startupCommands = [], $startupResponses = []) {
  $descriptorspec = array(
     0 => array("pipe", "r"), // stdin
     1 => array("pipe", "w"), // stdout
     2 => array("pipe", "w")  // stderr
  );

  // Grab an available port.
  $sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
  socket_bind($sock, "localhost", 0) or die("Could not bind socket");
  socket_getsockname($sock, $ip, $port);
  socket_close($sock);

  $addr = gethostbyname("localhost");
  $cmd = getHhvmPath() . " --mode vsdebug --vsDebugPort $port " . $scriptPath;
  $process = proc_open($cmd, $descriptorspec, $pipes);
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
    $tries++;
    sleep(1);
  }

  $GLOBALS['client'] = $client;

  // Upon connecting, expect a loader break message for the only thread.
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array("type" => "event", "event" => "stopped"));
  checkObjEqualRecursively($msg{'body'}, array("threadId" => 1, "reason" => "execution paused", "allThreadsStopped" => true));

  if ($sendDefaultInit) {
    // Send an initialize command.
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
    checkObjEqualRecursively($msg, array("type" => "response", "command" => "initialize", "success" => true, "request_seq" => 1));

    $launchCommand = array(
      "command" => "launch",
      "type" => "request",
      "seq" => 2,
      "arguments" => array(
        "startupDocumentPath" => __DIR__ . '/dummy.php.inc')
      );

    sendVsCommand($launchCommand);

    // Expect a launch response
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "response", "command" => "launch", "success" => true, "request_seq" => 2));

    // Expect an initialized event
    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "event", "event" => "initialized"));

    // There will be messages about the PHP/Hack console being prepared, skip those
    // since we don't care about them here. There is a test that checks those
    // messages explicitly.
    $msg = json_decode(getNextVsDebugMessage(), true);
    $msg = json_decode(getNextVsDebugMessage(), true);

    for ($i = 0; $i < count($startupCommands); ++$i) {
      sendVsCommand($startupCommands[$i]);
    }

    for ($i = 0; $i < count($startupResponses); ++$i) {
      $msg = json_decode(getNextVsDebugMessage(), true);
      checkObjEqualRecursively($msg, $startupResponses[$i]);
    }

    $configDoneCommand = array(
      "command" => "configurationDone",
      "type" => "request",
      "seq" => 3,
    );
    sendVsCommand($configDoneCommand);

    $msg = json_decode(getNextVsDebugMessage(), true);
    checkObjEqualRecursively($msg, array("type" => "response", "command" => "configurationDone", "success" => true, "request_seq" => 3));

    // Verify that the dummy was started. It echo'ed "I am the dummy!\n" to stdout if it did.
    $dummyMsg = "I am the dummy!\n";
    $result = fread($pipes[1], strlen($dummyMsg));
    if ($result !== $dummyMsg) {
      throw new UnexpectedValueException("$result !== $dummyMsg");
    }
  }

  return [$process, $pipes, $client];
}

function vsDebugCleanup($process, $pipes, $client, $expectedExitCode = 0) {
  $stdout = $pipes[1];
  $stderr = $pipes[2];
  echo stream_get_contents($stdout);
  echo stream_get_contents($stderr);

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
