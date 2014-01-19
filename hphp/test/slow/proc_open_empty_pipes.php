<?php

$command = 'echo "foo" >> /dev/null';
$descriptors = array(
  array('pipe', 'r'),
  array('pipe', 'w'),
);

$pipes = array(1, 2, 3, 4, 5);
$proc = proc_open($command, $descriptors, $pipes);
var_dump($proc);
var_dump($pipes);
