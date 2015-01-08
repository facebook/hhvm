<?php

if ($argc > 1) {
  echo "---- " . ini_get('variables_order') . " ----\n";

  var_dump(count($_SERVER) > 0);
  var_dump(count($_ENV) > 0);
  exit;
}

$args = array('', 'G', 'P', 'E', 'S', 'ES', 'ESCGP', 'CG');

foreach($args as $arg) {
  $descriptorspec = array(
     0 => array('pipe', 'r'),
     1 => array('pipe', 'w'),
     2 => array('pipe', 'w'));

  // -dvariables_order=  didn't consist with PHP
  // it should be fixed in ini level. so compatible with it for now
  $arg_str = $arg ? " -dvariables_order=$arg " : " ";

  $cmd = PHP_BINARY . $arg_str . __FILE__ . " dummy";

  $proc = proc_open($cmd, $descriptorspec, $pipes);
  if (!$proc) {
    echo "Failed to open: $cmd\n";
    exit -1;
  }

  echo stream_get_contents($pipes[1]);
  fclose($pipes[1]);

  proc_close($proc);
}
