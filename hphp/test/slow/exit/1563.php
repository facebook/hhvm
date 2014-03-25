<?php

function foo() {
  echo "a\n";
  exit(1);
}

function main() {
  var_dump(pcntl_signal(SIGUSR1, 'foo'));
  $pid = posix_getpid();
  posix_kill($pid, SIGUSR1);

  // Wait for signal to arrive.
  while (true) continue;
}

main();
