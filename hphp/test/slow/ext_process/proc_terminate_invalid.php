<?php
$cmd = 'sleep 3';
$descriptors = array();
$process = proc_open($cmd, $descriptors, $pipes);
$signal = 'Céphalopodes';
$result = @proc_terminate($process, $signal);
var_dump($result);
