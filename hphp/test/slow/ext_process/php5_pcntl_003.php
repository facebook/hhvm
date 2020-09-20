<?hh

// This is more or less a copy of zend/bad/ext/pcntl/tests/003.php.
// 003.php.skipif is overly-cautious, so it would skip that test.  This test
// also makes less assumptions about what the blocked signal set looks like
// coming into the test.
<<__EntryPoint>> function main(): void {
$old = null;
$first_set = null;
pcntl_sigprocmask(SIG_SETMASK, varray[], inout $old);

pcntl_sigprocmask(SIG_BLOCK, varray[SIGCHLD,SIGTERM], inout $first_set);

pcntl_sigprocmask(SIG_BLOCK, varray[SIGINT], inout $old);
var_dump(count($old) - count($first_set));

pcntl_sigprocmask(SIG_UNBLOCK, varray[SIGINT], inout $old);
var_dump(count($old) - count($first_set));

pcntl_sigprocmask(SIG_SETMASK, varray[SIGINT], inout $old);
var_dump(count($old) - count($first_set));

pcntl_sigprocmask(SIG_SETMASK, varray[], inout $old);
var_dump(count($old));

pcntl_sigprocmask(SIG_SETMASK, varray[], inout $old);
var_dump(count($old));
}
