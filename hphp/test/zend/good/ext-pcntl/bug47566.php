<?php
$pid = pcntl_fork();
if ($pid == -1) {
 echo "Unable to fork";
 exit;
} elseif ($pid) {
 $epid = pcntl_waitpid(-1,$status);
 var_dump(pcntl_wexitstatus($status));
} else {
 exit(128);
}
?>