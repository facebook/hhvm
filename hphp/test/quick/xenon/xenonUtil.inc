<?hh

function dumpFrame($frame, $name) {
  if ($frame) {
    echo $name . "Frame \n";
    foreach ($frame as $f) {
      $function = idx($f, "function", "");
      $file = idx($f, "file", "");
      $line = idx($f, "line", 0);
      printf("  (%d %s) %s\n", $line, $file, $function);
    }
  }
}

function xenonDump($stacks) {
  foreach ($stacks as $k => $v) {
    $ioWaitSample = ($v["ioWaitSample"]) ? "ioWaitSample" : "";
    echo "Frame " . $k . "  " . $ioWaitSample . "\n";
    $frame = $v["asyncStack"];
    dumpFrame($v["asyncStack"], "async");
    dumpFrame($v["phpStack"], "php");
    echo "\n";
  }
}

// get the Xenon data then verify that there are no unknown functions
// and that all of the functions in this file are in the stack
function verifyTestRun($stacks, $functionList, $requiredFunctions,
    $asyncList, $requiredAsync) {
  echo "Verifying PHP Stack\n";
  foreach ($stacks as $k => $v) {
    if (is_numeric($k)) {
      $frame = $v["phpStack"];
      if ($frame) {
        foreach ($frame as $f) {
          $function = idx($f, "function", "");
          $file = idx($f, "file", "");
          $line = idx($f, "line", 0);
          if (in_array($function, $functionList)) {
            $foundFunction = idx($requiredFunctions, $function, "");
            if ($foundFunction) {
              unset($requiredFunctions[$function]);
            }
          } else if (strpos($function, "Exception") === false
              && strpos($function, "create") === false) {
            echo "Unknown function:  " . $function . " " . $file . " "
              . $line . "\n";
          } // else meaningless line, skip it
        } // foreach
      } // if ($frame)
    } // if is_numeric
  }
  if ($requiredFunctions) {
    echo "Functions missing from stacks:  ";
    var_dump($requiredFunctions);
  }

  echo "Verifying Async Stack\n";
  foreach ($stacks as $k => $v) {
    if (is_numeric($k)) {
      $frame = $v["asyncStack"];
      if ($frame) {
        foreach ($frame as $f) {
          $function = idx($f, "function", "");
          $file = idx($f, "file", "");
          $line = idx($f, "line", 0);
          if (in_array($function, $asyncList)) {
            $foundFunction = idx($requiredAsync, $function, "");
            if ($foundFunction) {
              unset($requiredAsync[$function]);
            }
          } // else meaningless line, skip it
        } // foreach
      } // if ($frame)
    } // if is_numeric
  }
  if ($requiredAsync) {
    echo "Functions missing from stacks:  ";
    var_dump($requiredAsync);
  }

  echo "Finished verfying stacks\n";
}
