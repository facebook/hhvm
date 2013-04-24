<?php
$descriptors = array(
    0 => array('pipe', 'r'),
    1 => array('pipe', 'w'),
    2 => array('pipe', 'w'));

$pipes = array();

$process = proc_open('/bin/sleep 120', $descriptors, $pipes);

proc_terminate($process, 9);
sleep(1); // wait a bit to let the process finish
var_dump(proc_get_status($process));

echo "Done!\n";

?>