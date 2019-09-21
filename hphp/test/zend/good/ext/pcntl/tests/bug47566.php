<?hh <<__EntryPoint>> function main(): void {
$pid = pcntl_fork();
if ($pid == -1) {
 echo "Unable to fork";
 exit;
} elseif ($pid) {
 $status = null;
 $epid = pcntl_waitpid(-1, inout $status);
 var_dump(pcntl_wexitstatus($status));
} else {
 exit(128);
}
}
