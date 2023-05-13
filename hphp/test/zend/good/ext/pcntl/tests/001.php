<?hh

function test_exit_waits(){
  print "\n\nTesting pcntl_wifexited and wexitstatus....";

  $pid=pcntl_fork();
  if ($pid==0) {
    sleep(1);
    exit(100);
  } else if ($pid < 0) {
    print "\nUnable to fork";
    exit(-1);
  } else {
    $options=0;
    $status = null;
    if (pcntl_waitpid($pid, inout $status, $options) !== $pid) {
      print "\nUnable to wait on " . $pid;
      exit(-1);
    }
    if (pcntl_wifexited($status)) {
      print "\nExited With: ". pcntl_wexitstatus($status);
    } else {
      print "\nProcess did not exit";
    }
  }
}

function test_exit_signal(){
  print "\n\nTesting pcntl_wifsignaled....";

  $pid=pcntl_fork();
  if ($pid==0) {
    sleep(60);
    exit(101);
  } else if ($pid < 0) {
    print "\nUnable to fork";
    exit(-1);
  } else {
    $options=0;
    if (!posix_kill($pid, SIGTERM)) {
      print "\nUnable to send SIGTERM to " . $pid;
      exit(-1);
    }
    $status = null;
    if (pcntl_waitpid($pid, inout $status, $options) !== $pid) {
      print "\nUnable to wait on " . $pid;
      exit(-1);
    }
    if (pcntl_wifsignaled($status)) {
      $signal_print=pcntl_wtermsig($status);
      if ($signal_print==SIGTERM) $signal_print="SIGTERM";
      print "\nProcess was terminated by signal : ". $signal_print;
    } else if (pcntl_wifexited($status)) {
      print "\nProcess exited with: " . pcntl_wexitstatus($status);
    } else {
      print "\nProcess was not terminated";
    }
  }
}

function test_stop_signal(){
  print "\n\nTesting pcntl_wifstopped and pcntl_wstopsig....";

  $pid=pcntl_fork();
  if ($pid==0) {
    sleep(60);
    exit(102);
  } else if ($pid < 0) {
    print "\nUnable to fork";
    exit(-1);
  } else {
    $options=WUNTRACED;
    if (!posix_kill($pid, SIGSTOP)) {
      print "\nUnable to send SIGSTOP to " . $pid;
      exit(-1);
    }
    $status = null;
    if (pcntl_waitpid($pid, inout $status, $options) !== $pid) {
      print "\nUnable to wait on " . $pid;
    }
    if (pcntl_wifstopped($status)) {
      $signal_print=pcntl_wstopsig($status);
      if ($signal_print==SIGSTOP) $signal_print="SIGSTOP";
      print "\nProcess was stopped by signal : ". $signal_print;
    } else if (pcntl_wifexited($status)) {
      print "\nProcess exited with: " . pcntl_wexitstatus($status);
    } else {
      print "\nProcess was not stopped";
    }
    posix_kill($pid, SIGCONT);
  }
}

<<__EntryPoint>> function main(): void {
  print "Starting tests....";
  test_exit_waits();
  test_exit_signal();
  test_stop_signal();
}
