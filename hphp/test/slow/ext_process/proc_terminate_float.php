<?php
$cmd = 'sleep 3';
$descriptors = array();
$process = proc_open($cmd, $descriptors, $pipes);
$signal = 4.5;
$result = @proc_terminate($process, $signal);
var_dump($result);
