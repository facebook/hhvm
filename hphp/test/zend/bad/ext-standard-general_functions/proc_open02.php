<?php
$ds = array(array('pipe', 'r'));

$cat = proc_open(
	'/bin/sleep 2',
	$ds,
	$pipes
);

usleep(20000); // let the OS run the sleep process before sending the signal

var_dump(proc_terminate($cat, 0)); // status check
usleep(20000);
var_dump(proc_get_status($cat));

var_dump(proc_terminate($cat)); // now really quit it
usleep(20000);
var_dump(proc_get_status($cat));

proc_close($cat);

echo "Done!\n";

?>