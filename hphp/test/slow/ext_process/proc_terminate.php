<?php
$cmd = 'sleep 3';
$descriptors = array();

// Don't add signals that would cause the process to abort, the output will
// depend on whether it dumps its core.
$signals = array(
  2.2,    // SIGINT
  '9',    // SIGKILL
  'herp', // invalid, does nothing
  -4,     // invalid
);

foreach ($signals as $signal) {
  $process = proc_open($cmd, $descriptors, $pipes);
  $result = proc_terminate($process, $signal);
  var_dump($result);
}
