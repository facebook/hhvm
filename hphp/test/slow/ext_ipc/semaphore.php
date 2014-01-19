<?php

$ret = sem_get(0xDEADBEEF);
if ($ret === false) { echo "failed\n"; exit(1); }
$sem = $ret;
$now = microtime(true);
var_dump(sem_acquire($sem));

$pid = pcntl_fork();
if ($pid == 0) {
  $sem = sem_get(0xDEADBEEF);
  if (sem_acquire($sem) !== true) {
    echo "oops1\n";
  }

  // This isn't a sure test, but may be false if sem_acquire() didn't work
  $then = microtime(true);
  if (!($then - $now > 1)) {
    echo "oops: $then $now\n";
  }

  sem_release($sem);
  sem_remove($sem);
  exit(0);
}

sleep(3); // aha
sem_release($sem);
pcntl_waitpid($pid, $status);
