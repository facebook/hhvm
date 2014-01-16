<?php
pcntl_signal(SIGALRM, function(){});

var_dump(pcntl_alarm());
pcntl_alarm(0);
var_dump(pcntl_alarm(60));
var_dump(pcntl_alarm(1) > 0);
$siginfo = array();
var_dump(pcntl_sigtimedwait(array(SIGALRM),$siginfo,2) === SIGALRM);
?>