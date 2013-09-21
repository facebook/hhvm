<?php

echo "*** Test by calling function with pid error ***\n";

$pid = 10000;

do {
  $pid += 1;   	
  $result = shell_exec("ps -p " . $pid);
} while (strstr($pid, $result)); 

posix_kill($pid, SIGKILL);
var_dump(posix_errno());

?>