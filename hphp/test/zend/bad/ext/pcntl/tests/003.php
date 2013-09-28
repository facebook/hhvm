<?php

pcntl_sigprocmask(SIG_BLOCK, array(SIGCHLD,SIGTERM), $old);
var_dump(count($old));
pcntl_sigprocmask(SIG_BLOCK, array(SIGINT), $old);
var_dump(count($old));
pcntl_sigprocmask(SIG_UNBLOCK, array(SIGINT), $old);
var_dump(count($old));
pcntl_sigprocmask(SIG_SETMASK, array(SIGINT), $old);
var_dump(count($old));
pcntl_sigprocmask(SIG_SETMASK, array(), $old);
var_dump(count($old));
pcntl_sigprocmask(SIG_SETMASK, array(), $old);
var_dump(count($old));

?>