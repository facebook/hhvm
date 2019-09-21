<?hh

SignalChangeHandlerAfterForkPhp::$parent = posix_getpid();
SignalChangeHandlerAfterForkPhp::$child = 0;

// parent and child processes share the same handler here
function handler($signo) {

  if ($signo == SIGUSR2) {
    if (SignalChangeHandlerAfterForkPhp::$child == 0) {
      echo "child: I received SIGUSR2...\n";
      echo "child: sending SIGUSR1 to inform parent\n";
      posix_kill(SignalChangeHandlerAfterForkPhp::$parent, SIGUSR1);
      exit;
    } else {
      echo "parent: I received SIGUSR2, switching to new handler\n";
      pcntl_signal(SIGUSR1, "newhandler");
      pcntl_signal(SIGUSR2, "newhandler");
      echo "parent: sending SIGUSR2 to child as ack\n";
      posix_kill(SignalChangeHandlerAfterForkPhp::$child, SIGUSR2);
    }
  } else if ($signo == SIGUSR1) {
    if (SignalChangeHandlerAfterForkPhp::$child != 0) {
      echo "parent: received SIGUSR1 from child\n";
      $status = null;
      pcntl_waitpid(SignalChangeHandlerAfterForkPhp::$child, inout $status);
      echo "child exit with code $status";
      exit(0);
    }
  }
}

function newhandler($signo) {

  echo "parent: new handler invoked\n";
  $status = null;
  pcntl_waitpid(SignalChangeHandlerAfterForkPhp::$child, inout $status);
  exit(0);
}

function waitForSignal($nsec) {
  for ($i = 0; $i < $nsec; ++$i) {
    sleep(1);
  }
}

function main() {
  pcntl_signal(SIGUSR1, fun("handler"));
  pcntl_signal(SIGUSR2, fun("handler"));

  SignalChangeHandlerAfterForkPhp::$child = pcntl_fork();  // 0 in the child process

  if (SignalChangeHandlerAfterForkPhp::$child == 0) {
    echo "do some work in child process\n";
    posix_kill(SignalChangeHandlerAfterForkPhp::$parent, SIGUSR2);
    waitForSignal(10);
  } else {
    waitForSignal(10);
  }

  echo "this shouldn't be printed.";
}

main();

abstract final class SignalChangeHandlerAfterForkPhp {
  public static $parent;
  public static $child;
}
