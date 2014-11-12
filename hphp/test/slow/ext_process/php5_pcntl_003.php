<?php
// This is a copy of zend/bad/ext/pcntl/tests/003.php
// 003.php.skipif is overly-cautious, so skips this test

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
