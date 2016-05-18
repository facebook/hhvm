<?php

// This is more or less a copy of zend/bad/ext/pcntl/tests/003.php.
// 003.php.skipif is overly-cautious, so it would skip that test.  This test
// also makes less assumptions about what the blocked signal set looks like
// coming into the test.

pcntl_sigprocmask(SIG_BLOCK, array(SIGCHLD,SIGTERM), $first_set);

pcntl_sigprocmask(SIG_BLOCK, array(SIGINT), $old);
var_dump(count($old) - count($first_set));

pcntl_sigprocmask(SIG_UNBLOCK, array(SIGINT), $old);
var_dump(count($old) - count($first_set));

pcntl_sigprocmask(SIG_SETMASK, array(SIGINT), $old);
var_dump(count($old) - count($first_set));

pcntl_sigprocmask(SIG_SETMASK, array(), $old);
var_dump(count($old));

pcntl_sigprocmask(SIG_SETMASK, array(), $old);
var_dump(count($old));

?>
