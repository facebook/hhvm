<?hh

function dumpFrame($frame, $name): void {
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

function xenonDump($stacks): void {
  foreach ($stacks as $k => $v) {
    $ioWaitSample = ($v["ioWaitSample"]) ? "ioWaitSample" : "";
    echo "Frame " . $k . "  " . $ioWaitSample . "\n";
    dumpFrame($v["phpStack"], "php");
    echo "\n";
  }
}

// get the Xenon data then verify that there are no unknown functions
// and that all of the functions in this file are in the stack
function verifyTestRun($stacks, $required, $optional): void {
  echo "Verifying PHP Stack\n";
  $required = array_fill_keys($required, true);
  $required += array_fill_keys($optional, false);
  foreach ($stacks as $k => $v) {
    if (!is_numeric($k)) {
      continue;
    }

    $frame = $v["phpStack"];
    if (!$frame) {
      continue;
    }

    foreach ($frame as $f) {
      $function = idx($f, 'function', '');
      if (array_key_exists($function, $required)) {
        $required[$function] = false;
      } else {
        $file = idx($f, "file", "");
        $line = idx($f, "line", 0);
        echo sprintf(
          "Unknown function:  %s %s %s\n",
          $function,
          idx($f, 'file', ''),
          idx($f, 'line', 0),
        );
      }
    }
  }

  $required = array_filter($required);
  if ($required) {
    echo "Functions missing from stacks:  ";
    var_dump(array_keys($required));
  }

  echo "Finished verifying stacks\n";
}
