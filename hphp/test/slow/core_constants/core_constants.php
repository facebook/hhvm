<?hh

// This test is currently *Linux only* due
// to the use of posix_getpid() and
// readlink. This will not work on Windows.
// This may work on FreeBSD but has not been
// tested.
<<__EntryPoint>>
function main_core_constants() {
$pid = posix_getpid();
$output = null;
$return_var = -1;
$exe = exec("readlink -f /proc/$pid/exe", inout $output, inout $return_var);
var_dump($exe === HH\__internal\hhvm_binary());
}
