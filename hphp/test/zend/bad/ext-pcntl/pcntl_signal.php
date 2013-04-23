<?php
pcntl_signal(SIGTERM, function($signo){
	echo "signal dispatched\n";
});
posix_kill(posix_getpid(), SIGTERM);
pcntl_signal_dispatch();

var_dump(pcntl_signal());
var_dump(pcntl_signal(SIGALRM, SIG_IGN));
var_dump(pcntl_signal(-1, -1));
var_dump(pcntl_signal(-1, function(){}));
var_dump(pcntl_signal(SIGALRM, "not callable"));


/* test freeing queue in RSHUTDOWN */
posix_kill(posix_getpid(), SIGTERM);
echo "ok\n";
?>