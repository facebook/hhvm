<?hh
function test_exit_waits(){
    print "\n\nTesting pcntl_wifexited and wexitstatus....";

    $pid=pcntl_fork();
    if ($pid==0) {
        sleep(1);
        exit(-1);
    } else {
        $options=0;
        $status = null;
        pcntl_waitpid($pid, inout $status, $options);
        if ( pcntl_wifexited($status) ) print "\nExited With: ". pcntl_wexitstatus($status);
    }
}

function test_exit_signal(){
    print "\n\nTesting pcntl_wifsignaled....";

    $pid=pcntl_fork();

    if ($pid==0) {
        sleep(10);
        exit;
    } else {
        $options=0;
        posix_kill($pid, SIGTERM);
        $status = null;
        pcntl_waitpid($pid, inout $status, $options);
        if ( pcntl_wifsignaled($status) ) {
            $signal_print=pcntl_wtermsig($status);
            if ($signal_print==SIGTERM) $signal_print="SIGTERM";
            print "\nProcess was terminated by signal : ". $signal_print;
        }

    }
}


function test_stop_signal(){
    print "\n\nTesting pcntl_wifstopped and pcntl_wstopsig....";

    $pid=pcntl_fork();

    if ($pid==0) {
        sleep(1);
        exit;
    } else {
        $options=WUNTRACED;
        posix_kill($pid, SIGSTOP);
        $status = null;
        pcntl_waitpid($pid, inout $status, $options);
        if ( pcntl_wifstopped($status) ) {
            $signal_print=pcntl_wstopsig($status);
            if ($signal_print==SIGSTOP) $signal_print="SIGSTOP";
            print "\nProcess was stoped by signal : ". $signal_print;
        }
        posix_kill($pid, SIGCONT);
    }
}
<<__EntryPoint>> function main(): void {
print "Staring wait.h tests....";
test_exit_waits();
test_exit_signal();
test_stop_signal();
}
