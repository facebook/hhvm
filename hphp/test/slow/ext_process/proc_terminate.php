<?php
$cmd = 'sleep 3';
$descriptors = array();
$signals = array(4.5, '0', 'Céphalopodes', -4);

foreach ($signals as $signal) {
  $process = proc_open($cmd, $descriptors, $pipes);
  $result = @proc_terminate($process, $signal);
  var_dump($result);
}
