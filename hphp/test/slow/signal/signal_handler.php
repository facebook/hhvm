<?php

$signo = 0;

function handler($signo) {
  switch ($signo) {
    case SIGUSR1:
      echo "received SIGUSR1\n";
      break;
    case SIGUSR2:
      echo "received SIGUSR2\n";
      break;
    case SIGTERM:
      echo "received SIGTERM, exiting\n";
      exit;
    default:
      echo "what?";
  }
}

pcntl_signal(SIGUSR1, "handler");
pcntl_signal(SIGUSR2, "handler");
pcntl_signal(SIGTERM, "handler");

$pid = posix_getpid();
for ($i = 0; $i < 5; ++$i) {
  posix_kill($pid, SIGUSR2);
  pcntl_signal_dispatch();
  posix_kill($pid, SIGUSR1);
  pcntl_signal_dispatch();
}
posix_kill($pid, SIGTERM);
pcntl_signal_dispatch();

echo "should not be printed\n";
