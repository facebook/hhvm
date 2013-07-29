<?php
// This test is currently *Linux only* due
// to the use of posix_getpid() and
// readlink. This will not work on Windows.
// This may work on FreeBSD but has not been
// tested.
$pid = posix_getpid();
$exe = exec("readlink -f /proc/$pid/exe");
var_dump($exe === PHP_BINARY);
$i = strrpos($exe, "/");
var_dump(substr($exe, 0, $i) === PHP_BINDIR);
