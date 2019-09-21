<?hh <<__EntryPoint>> function main(): void {
echo "*** Test by calling method or function with its expected arguments, first print the child PID and the father ***\n";

$pid = pcntl_fork();
if ($pid > 0) {
    $status = null;
    pcntl_wait(inout $status);
    var_dump($pid);
} else {
    var_dump($pid);
}
}
