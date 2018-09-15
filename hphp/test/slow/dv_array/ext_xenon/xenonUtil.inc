<?hh

// get the Xenon data then verify that there are no unknown functions
// and that all of the functions in this file are in the stack
function verifyTestRun($samples, $required, $optional): void {
  echo "Verifying PHP Stack\n";
  $required = array_fill_keys($required, true);
  $required += array_fill_keys($optional, false);
  foreach ($samples as $sample) {
    $stack = $sample['stack'];
    if (!$stack) {
      continue;
    }

    foreach ($stack as $frame) {
      $cls = $frame['class'] ?? '';
      $function = $frame['function'] ?? '';
      $function = $cls ? $cls.'::'.$function : $function;
      if (array_key_exists($function, $required)) {
        $required[$function] = false;
      } else {
        $file = $frame['file'] ?? '';
        $line = $frame['line'] ?? 0;
        echo sprintf(
          "Unexpected function in call stack:  %s %s %s\n",
          $function,
          $file,
          $line,
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
